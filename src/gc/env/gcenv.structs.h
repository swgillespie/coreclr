// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
#ifndef __GCENV_STRUCTS_INCLUDED__
#define __GCENV_STRUCTS_INCLUDED__
//
// Structs shared between the GC and the environment
//

struct GCSystemInfo
{
    uint32_t dwNumberOfProcessors;
    uint32_t dwPageSize;
    uint32_t dwAllocationGranularity;
};

enum HeapVerifyFlags {
};

class EEConfig {
public:
    enum {
        HEAPVERIFY_NONE             = 0,
        HEAPVERIFY_GC               = 1,   // Verify the heap at beginning and end of GC
        HEAPVERIFY_BARRIERCHECK     = 2,   // Verify the brick table
        HEAPVERIFY_SYNCBLK          = 4,   // Verify sync block scanning

        // the following options can be used to mitigate some of the overhead introduced
        // by heap verification.  some options might cause heap verifiction to be less
        // effective depending on the scenario.

        HEAPVERIFY_NO_RANGE_CHECKS  = 0x10,   // Excludes checking if an OBJECTREF is within the bounds of the managed heap
        HEAPVERIFY_NO_MEM_FILL      = 0x20,   // Excludes filling unused segment portions with fill pattern
        HEAPVERIFY_POST_GC_ONLY     = 0x40,   // Performs heap verification post-GCs only (instead of before and after each GC)
        HEAPVERIFY_DEEP_ON_COMPACT  = 0x80    // Performs deep object verfication only on compacting GCs.
    };

    enum  GCStressFlags {
        GCSTRESS_NONE = 0,
        GCSTRESS_ALLOC = 1,    // GC on all allocs and 'easy' places
        GCSTRESS_TRANSITION = 2,    // GC on transitions to preemptive GC
        GCSTRESS_INSTR_JIT = 4,    // GC on every allowable JITed instr
        GCSTRESS_INSTR_NGEN = 8,    // GC on every allowable NGEN instr
        GCSTRESS_UNIQUE = 16,   // GC only on a unique stack trace
    };

    int GetHeapVerifyLevel();
    size_t GetSegmentSize();
    int GetGCLatencyMode();
    bool GetGCRetainVM();
    bool GetGCconcurrent();
    int GetGCprnLvl();
    int GetGCtraceFac();
    int GetGCLOHCompactionMode();
    bool IsGCBreakOnOOMEnabled();
    GCStressFlags GetGCStressLevel();
    bool IsGCStressMix();
    int GetGCtraceStart();
    int GetGCtraceEnd();
    bool GetGCConservative();
    bool GetGCForceCompact();
    bool GetGCNoAffinitize();
    int GetGCHeapCount();
    int GetFastGCStressLevel();
    int GetGCStressStep();
    bool GetAppDomainLeaks();
    int GetGCgen0size();
};

typedef void * HANDLE;

#ifdef PLATFORM_UNIX

typedef char TCHAR;
#define _T(s) s

#else

#ifndef _INC_WINDOWS
typedef wchar_t TCHAR;
#define _T(s) L##s
#endif

#endif

#ifdef PLATFORM_UNIX

class EEThreadId
{
    pthread_t m_id;
    // Indicates whether the m_id is valid or not. pthread_t doesn't have any
    // portable "invalid" value.
    bool m_isValid;

public:
    bool IsCurrentThread()
    {
        return m_isValid && pthread_equal(m_id, pthread_self());
    }

    void SetToCurrentThread()
    {
        m_id = pthread_self();
        m_isValid = true;
    }

    void Clear()
    {
        m_isValid = false;
    }
};

#else // PLATFORM_UNIX

#ifndef _INC_WINDOWS
extern "C" uint32_t __stdcall GetCurrentThreadId();
#endif

class EEThreadId
{
    uint64_t m_uiId;
public:

    bool IsCurrentThread()
    {
        return m_uiId == ::GetCurrentThreadId();
    }

    void SetToCurrentThread()
    {
        m_uiId = ::GetCurrentThreadId();
    }

    void Clear()
    {
        m_uiId = 0;
    }
};

#endif // PLATFORM_UNIX

#ifndef _INC_WINDOWS

#ifdef PLATFORM_UNIX

typedef struct _RTL_CRITICAL_SECTION {
    pthread_mutex_t mutex;
} CRITICAL_SECTION, RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;

#else

#pragma pack(push, 8)

typedef struct _RTL_CRITICAL_SECTION {
    void* DebugInfo;

    //
    //  The following three fields control entering and exiting the critical
    //  section for the resource
    //

    int32_t LockCount;
    int32_t RecursionCount;
    HANDLE OwningThread;        // from the thread's ClientId->UniqueThread
    HANDLE LockSemaphore;
    uintptr_t SpinCount;        // force size on 64-bit systems when packed
} CRITICAL_SECTION, RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;

#pragma pack(pop)

#endif

#endif // _INC_WINDOWS

#endif // __GCENV_STRUCTS_INCLUDED__
