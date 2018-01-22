// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#include "common.h"
#include "gcheaputilities.h"
#include "gcenv.ee.h"
#include "appdomain.hpp"


// These globals are variables used within the GC and maintained
// by the EE for use in write barriers. It is the responsibility
// of the GC to communicate updates to these globals to the EE through
// GCToEEInterface::StompWriteBarrierResize and GCToEEInterface::StompWriteBarrierEphemeral.
GPTR_IMPL_INIT(uint32_t, g_card_table,      nullptr);
GPTR_IMPL_INIT(uint8_t,  g_lowest_address,  nullptr);
GPTR_IMPL_INIT(uint8_t,  g_highest_address, nullptr);
GVAL_IMPL_INIT(GCHeapType, g_heap_type,     GC_HEAP_INVALID);
uint8_t* g_ephemeral_low  = (uint8_t*)1;
uint8_t* g_ephemeral_high = (uint8_t*)~0;

#ifdef FEATURE_MANUALLY_MANAGED_CARD_BUNDLES
uint32_t* g_card_bundle_table = nullptr;
#endif

// This is the global GC heap, maintained by the VM.
GPTR_IMPL(IGCHeap, g_pGCHeap);

GcDacVars g_gc_dac_vars;
GPTR_IMPL(GcDacVars, g_gcDacGlobals);

#ifdef FEATURE_USE_SOFTWARE_WRITE_WATCH_FOR_GC_HEAP

uint8_t* g_sw_ww_table = nullptr;
bool g_sw_ww_enabled_for_gc_heap = false;

#endif // FEATURE_USE_SOFTWARE_WRITE_WATCH_FOR_GC_HEAP

gc_alloc_context g_global_alloc_context = {};

enum GC_LOAD_STATUS {
    GC_LOAD_STATUS_BEFORE_START,
    GC_LOAD_STATUS_START,
    GC_LOAD_STATUS_DONE_LOAD,
    GC_LOAD_STATUS_GET_VERSIONINFO,
    GC_LOAD_STATUS_CALL_VERSIONINFO,
    GC_LOAD_STATUS_DONE_VERSION_CHECK,
    GC_LOAD_STATUS_GET_INITIALIZE,
    GC_LOAD_STATUS_LOAD_COMPLETE
};

// Load status of the GC. If GC loading fails, the value of this
// global indicates where the failure occured.
GC_LOAD_STATUS g_gc_load_status = GC_LOAD_STATUS_BEFORE_START;

// The version of the GC that we have loaded.
VersionInfo g_gc_version_info;

// The module that contains the GC.
HMODULE g_gc_module;

// GC entrypoints for the the linked-in GC. These symbols are invoked
// directly if we are not using a standalone GC.
extern "C" void GC_VersionInfo(/* Out */ VersionInfo* info);
extern "C" HRESULT GC_Initialize(
    /* In  */ IGCToCLR* clrToGC,
    /* Out */ IGCHeap** gcHeap,
    /* Out */ IGCHandleManager** gcHandleManager,
    /* Out */ GcDacVars* gcDacVars
);

#ifndef DACCESS_COMPILE

HMODULE GCHeapUtilities::GetGCModule()
{
    assert(g_gc_module);
    return g_gc_module;
}

namespace
{

// This block of code contains all of the state necessary to handle incoming
// EtwCallbacks before the GC has been initialized. This is a tricky problem
// because EtwCallbacks can appear at any time, even when we are just about
// finished initializing the GC.
//
// The below lock is taken by the "main" thread (the thread in EEStartup) and
// the "ETW" thread, the one calling EtwCallback. EtwCallback may or may not
// be called on the main thread.
CRITICAL_SECTION g_eventStashLock;

// These two global fields are 64-bit integers so that loads and stores to them
// can be atomic.
LONG64 g_stashedKeywordAndLevel = 0;
LONG64 g_stashedPrivateKeywordAndLevel = 0;

// In order for loads and stores of the keyword and level state to be atomic,
// these macros assist in packing and unpacking two 32-bit integer values into
// a 64-bit value.
#define STASH_KEY_AND_LEVEL(key, level) ((static_cast<LONG64>(key) << 32) | static_cast<LONG64>(level))
#define UNSTASH_KEY(combined) (static_cast<GCEventKeyword>(combined >> 32))
#define UNSTASH_VALUE(combined) (static_cast<GCEventLevel>(combined & 0xFFFFFFFF))

// FinalizeLoad is called by the main thread to complete initialization of the GC.
// At this point, the GC has provided us with an IGCHeap instance and we are preparing
// to "publish" it by assigning it to g_pGCHeap.
//
// This function can proceed concurrently with StashKeywordAndLevel below.
void FinalizeLoad(IGCHeap* gcHeap, IGCHandleManager* handleMgr, HMODULE gcModule)
{
    // at this point the GC has been initialized and is ready to go.
    // the main thread is responsible for initializing the lock.
    UnsafeInitializeCriticalSection(&g_eventStashLock);
    UnsafeEnterCriticalSection(&g_eventStashLock);

    // the compiler or hardware can't be allowed to reorder this write before
    // the lock acquision. This is because StashKeywordAndLevel will acquire the
    // lock if g_pGCHeap is not null and the lock must be initialized by that point.
    VOLATILE_MEMORY_BARRIER();
    g_pGCHeap = gcHeap;

    // despite holding the lock, the ETW callback thread may still write to the stashed
    // values. All writes are atomic.
    //
    // the values we read here may not necessarily be the most up-to-date.
    // Between the time we read these values and we call ControlEvents, an ETW callback
    // may have been fired and overwrote these values with more up-to-date ones.
    // However, if this is the case, the ETW callback is now blocked waiting to acquire
    // g_eventStashLock, so it will have a chance to call ControlEvents itself later
    // once we release the lock.
    LONG64 keyAndLevel = InterlockedExchange64(&g_stashedKeywordAndLevel, 0);
    LONG64 privateKeyAndLevel = InterlockedExchange64(&g_stashedPrivateKeywordAndLevel, 0);

    // Ultimately, g_eventStashLock ensures that no two threads call ControlEvents at any
    // point in time.
    g_pGCHeap->ControlEvents(UNSTASH_KEY(keyAndLevel), UNSTASH_VALUE(keyAndLevel));
    g_pGCHeap->ControlPrivateEvents(UNSTASH_KEY(privateKeyAndLevel), UNSTASH_VALUE(privateKeyAndLevel));
    UnsafeLeaveCriticalSection(&g_eventStashLock);

    g_pGCHandleManager = handleMgr;
    g_gcDacGlobals = &g_gc_dac_vars;
    g_gc_load_status = GC_LOAD_STATUS_LOAD_COMPLETE;
    g_gc_module = gcModule;
    LOG((LF_GC, LL_INFO100, "GC load successful\n"));
}

void StashKeywordAndLevel(bool isPublicProvider, GCEventKeyword keywords, GCEventLevel level)
{
    volatile LONG64* stash = isPublicProvider ? &g_stashedKeywordAndLevel : &g_stashedPrivateKeywordAndLevel;
    InterlockedExchange64(stash, STASH_KEY_AND_LEVEL(keywords, level));

    VOLATILE_MEMORY_BARRIER();
    if (g_pGCHeap != nullptr)
    {
        // observing that g_pGCHeap is not null means that g_eventStashLock has been
        // initialized. At this point either the GC is still being initialized
        // (the main thread will hold the lock) or the GC has already been initialized
        // (the lock will be uncontested).
        //
        // At any rate, once we grab the lock, we're free to inform the GC of our changes
        // to the event state.
        UnsafeEnterCriticalSection(&g_eventStashLock);
        if (isPublicProvider)
        {
            g_pGCHeap->ControlEvents(keywords, level);
        }
        else
        {
            g_pGCHeap->ControlPrivateEvents(keywords, level);
        }

        UnsafeLeaveCriticalSection(&g_eventStashLock);
    }

    // note that we will do nothing if g_pGCHeap is null. The main thread will read our stashed
    // event state when initializing the GC, so by virtue of writing to the event stash we
    // will ensure that the initializing GC will see the event state that we were just informed
    // of by an ETW callback.
}

// Loads and initializes a standalone GC, given the path to the GC
// that we should load. Returns S_OK on success and the failed HRESULT
// on failure.
//
// See Documentation/design-docs/standalone-gc-loading.md for details
// on the loading protocol in use here.
HRESULT LoadAndInitializeGC(LPWSTR standaloneGcLocation)
{
    LIMITED_METHOD_CONTRACT;

#ifndef FEATURE_STANDALONE_GC
    LOG((LF_GC, LL_FATALERROR, "EE not built with the ability to load standalone GCs"));
    return E_FAIL;
#else
    LOG((LF_GC, LL_INFO100, "Loading standalone GC from path %S\n", standaloneGcLocation));
    HMODULE hMod = CLRLoadLibrary(standaloneGcLocation);
    if (!hMod)
    {
        HRESULT err = GetLastError();
        LOG((LF_GC, LL_FATALERROR, "Load of %S failed\n", standaloneGcLocation));
        return err;
    }

    // a standalone GC dispatches virtually on GCToEEInterface, so we must instantiate
    // a class for the GC to use.
    IGCToCLR* gcToClr = new (nothrow) standalone::GCToEEInterface();
    if (!gcToClr)
    {
        return E_OUTOFMEMORY;
    }

    g_gc_load_status = GC_LOAD_STATUS_DONE_LOAD;
    GC_VersionInfoFunction versionInfo = (GC_VersionInfoFunction)GetProcAddress(hMod, "GC_VersionInfo");
    if (!versionInfo)
    {
        HRESULT err = GetLastError();
        LOG((LF_GC, LL_FATALERROR, "Load of `GC_VersionInfo` from standalone GC failed\n"));
        return err;
    }

    g_gc_load_status = GC_LOAD_STATUS_GET_VERSIONINFO;
    versionInfo(&g_gc_version_info);
    g_gc_load_status = GC_LOAD_STATUS_CALL_VERSIONINFO;

    if (g_gc_version_info.MajorVersion != GC_INTERFACE_MAJOR_VERSION)
    {
        LOG((LF_GC, LL_FATALERROR, "Loaded GC has incompatible major version number (expected %d, got %d)\n",
            GC_INTERFACE_MAJOR_VERSION, g_gc_version_info.MajorVersion));
        return E_FAIL;
    }

    if (g_gc_version_info.MinorVersion < GC_INTERFACE_MINOR_VERSION)
    {
        LOG((LF_GC, LL_INFO100, "Loaded GC has lower minor version number (%d) than EE was compiled against (%d)\n",
            g_gc_version_info.MinorVersion, GC_INTERFACE_MINOR_VERSION));
    }

    LOG((LF_GC, LL_INFO100, "Loaded GC identifying itself with name `%s`\n", g_gc_version_info.Name));
    g_gc_load_status = GC_LOAD_STATUS_DONE_VERSION_CHECK;
    GC_InitializeFunction initFunc = (GC_InitializeFunction)GetProcAddress(hMod, "GC_Initialize");
    if (!initFunc)
    {
        HRESULT err = GetLastError();
        LOG((LF_GC, LL_FATALERROR, "Load of `GC_Initialize` from standalone GC failed\n"));
        return err;
    }

    g_gc_load_status = GC_LOAD_STATUS_GET_INITIALIZE;
    IGCHeap* heap;
    IGCHandleManager* manager;
    HRESULT initResult = initFunc(gcToClr, &heap, &manager, &g_gc_dac_vars);
    if (initResult == S_OK)
    {
        FinalizeLoad(heap, manager, hMod);
    }
    else
    {
        LOG((LF_GC, LL_FATALERROR, "GC initialization failed with HR = 0x%X\n", initResult));
    }

    return initResult;
#endif // FEATURE_STANDALONE_GC
}

// Initializes a non-standalone GC. The protocol for initializing a non-standalone GC
// is similar to loading a standalone one, except that the GC_VersionInfo and
// GC_Initialize symbols are linked to directory and thus don't need to be loaded.
//
// The major and minor versions are still checked in debug builds - it must be the case
// that the GC and EE agree on a shared version number because they are built from
// the same sources.
HRESULT InitializeDefaultGC()
{
    LIMITED_METHOD_CONTRACT;

    LOG((LF_GC, LL_INFO100, "Standalone GC location not provided, using provided GC\n"));

    g_gc_load_status = GC_LOAD_STATUS_DONE_LOAD;
    VersionInfo info;
    GC_VersionInfo(&g_gc_version_info);
    g_gc_load_status = GC_LOAD_STATUS_CALL_VERSIONINFO;

    // the default GC builds with the rest of the EE. By definition, it must have been
    // built with the same interface version.
    assert(g_gc_version_info.MajorVersion == GC_INTERFACE_MAJOR_VERSION);
    assert(g_gc_version_info.MinorVersion == GC_INTERFACE_MINOR_VERSION);
    g_gc_load_status = GC_LOAD_STATUS_DONE_VERSION_CHECK;

    IGCHeap* heap;
    IGCHandleManager* manager;
    HRESULT initResult = GC_Initialize(nullptr, &heap, &manager, &g_gc_dac_vars);
    if (initResult == S_OK)
    {
        FinalizeLoad(heap, manager, GetModuleInst());
    }
    else
    {
        LOG((LF_GC, LL_FATALERROR, "GC initialization failed with HR = 0x%X\n", initResult));
    }


    return initResult;
}

} // anonymous namespace

// Loads (if necessary) and initializes the GC. If using a standalone GC,
// it loads the library containing it and dynamically loads the GC entry point.
// If using a non-standalone GC, it invokes the GC entry point directly.
HRESULT GCHeapUtilities::LoadAndInitialize()
{
    LIMITED_METHOD_CONTRACT;

    // we should only call this once on startup. Attempting to load a GC
    // twice is an error.
    assert(g_pGCHeap == nullptr);

    // we should not have attempted to load a GC already. Attempting a
    // load after the first load already failed is an error.
    assert(g_gc_load_status == GC_LOAD_STATUS_BEFORE_START);
    g_gc_load_status = GC_LOAD_STATUS_START;

    LPWSTR standaloneGcLocation = nullptr;
    CLRConfig::GetConfigValue(CLRConfig::EXTERNAL_GCName, &standaloneGcLocation);
    if (!standaloneGcLocation)
    {
        return InitializeDefaultGC();
    }
    else
    {
        return LoadAndInitializeGC(standaloneGcLocation);
    }
}

void GCHeapUtilities::RecordEventStateChange(bool isPublicProvider, GCEventKeyword keywords, GCEventLevel level)
{
    CONTRACTL {
      MODE_ANY;
      NOTHROW;
      GC_NOTRIGGER;
      CAN_TAKE_LOCK;
    } CONTRACTL_END;

    StashKeywordAndLevel(isPublicProvider, keywords, level);
}

#endif // DACCESS_COMPILE
