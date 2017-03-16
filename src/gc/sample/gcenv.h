// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

// The sample is to be kept simple, so building the sample
// in tandem with a standalone GC is currently not supported.
#ifdef FEATURE_STANDALONE_GC
#undef FEATURE_STANDALONE_GC
#endif // FEATURE_STANDALONE_GC

#if defined(_DEBUG)
#ifndef _DEBUG_IMPL
#define _DEBUG_IMPL 1
#endif
#define ASSERT(_expr) assert(_expr)
#else
#define ASSERT(_expr)
#endif

#ifndef _ASSERTE
#define _ASSERTE(_expr) ASSERT(_expr)
#endif

#include "gcenv.structs.h"
#include "gcenv.base.h"
#include "gcenv.os.h"
#include "gcenv.interlocked.h"
#include "gcenv.interlocked.inl"
#include "gcenv.object.h"
#include "gcenv.sync.h"
#include "gcenv.ee.h"

//
// Thread
//

struct alloc_context;

class Thread
{
    uint32_t m_fPreemptiveGCDisabled;
    uintptr_t m_alloc_context[16]; // Reserve enough space to fix allocation context

    friend class ThreadStore;
    Thread * m_pNext;

public:
    Thread()
    {
    }

    bool PreemptiveGCDisabled()
    {
        return !!m_fPreemptiveGCDisabled;
    }

    void EnablePreemptiveGC()
    {
        m_fPreemptiveGCDisabled = false;
    }

    void DisablePreemptiveGC()
    {
        m_fPreemptiveGCDisabled = true;
    }

    alloc_context* GetAllocContext()
    {
        return (alloc_context *)&m_alloc_context;
    }

    void SetGCSpecial(bool fGCSpecial)
    {
    }

    bool CatchAtSafePoint()
    {
        // This is only called by the GC on a background GC worker thread that's explicitly interested in letting
        // a foreground GC proceed at that point. So it's always safe to return true.
        return true;
    }
};

Thread * GetThread();

class ThreadStore
{
public:
    static Thread * GetThreadList(Thread * pThread);

    static void AttachCurrentThread();
};

#include "etmdummy.h"
