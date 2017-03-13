// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef __GCCONFIG_H__
#define __GCCONFIG_H__

#include "common.h"
#include "gc.h"

enum HeapVerifyFlags {
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

class GCConfig {
private:
    class Impl;
    Impl *_impl;

public:
    // Gets the current heap verify level as requested by the EE.
    int GetHeapVerifyLevel() const;

    // Gets the segment size, if requested by the EE.
    size_t GetSegmentSize() const;

    // Gets the initial GC latency mode.
    int GetGCLatencyMode() const;

    // Get whether or not we should be hoarding heap segments.
    bool GetGCRetainVM() const;

    // Get whether or not we should be using Server GC.
    bool GetGCConcurrent() const;

    int GetGCprnLvl() const;

    int GetGCtraceFac() const;

    int GetGCLOHCompactionMode() const;

    bool GetGCBreakOnOOMEnabled() const;

    int GetGCStressLevel() const;

    bool GetGCStressMix() const;

    int GetGCtraceStart() const;

    int GetGCtraceStop() const;

    bool GetGCConservative() const;

    bool GetGCForceCompact() const;

    bool GetGCNoAffinitize() const;

    int GetGCHeapCount() const;

    int GetFastGCStressLevel() const;

    int GetGCStressStep() const;

    bool GetAppDomainLeaks() const;

    int GetGCgen0Size() const;

    GCConfig();

    ~GCConfig();
};

// extern GCConfig* g_pConfig;

#endif // __GCCONFIG_H__
