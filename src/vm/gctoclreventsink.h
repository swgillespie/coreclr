// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef __GCTOCLREVENTSINK_H__
#define __GCTOCLREVENTSINK_H__

#include "gcinterface.h"

class GCToCLREventSink : public IGCToCLREventSink
{
public:
    void FireDynamicEvent(const char* eventName, void* payload, uint32_t payloadSize);
    void FireGCStart_V2(uint32_t count, uint32_t depth, uint32_t reason, uint32_t type);
    void FireGCEnd_V1(uint32_t count, uint32_t depth);
    void FireGCGenerationRange(uint8_t generation, void* rangeStart, uint64_t rangeUsedLength, uint64_t rangeReservedLength);
    void FireGCHeapStats_V1(
        uint64_t generationSize0,
        uint64_t totalPromotedSize0,
        uint64_t generationSize1,
        uint64_t totalPromotedSize1,
        uint64_t generationSize2,
        uint64_t totalPromotedSize2,
        uint64_t generationSize3,
        uint64_t totalPromotedSize3,
        uint64_t finalizationPromotedSize,
        uint64_t finalizationPromotedCount,
        uint32_t pinnedObjectCount,
        uint32_t sinkBlockCount,
        uint32_t gcHandleCount);
    void FireGCCreateSegment_V1(void* address, size_t size, uint32_t type);
    void FireGCFreeSegment_V1(void* address);
    void FireGCCreateConcurrentThread_V1();
    void FireGCTerminateConcurrentThread_V1();
    void FireGCTriggered(uint32_t reason);
    void FireGCMarkWithType(uint32_t heapNum, uint32_t type, uint64_t bytes);
    void FireGCJoin_V2(uint32_t heap, uint32_t joinTime, uint32_t joinType, uint32_t joinId);
    void FireGCGlobalHeapHistory_V2(uint64_t finalYoungestDesired,
        int32_t numHeaps,
        uint32_t condemnedGeneration,
        uint32_t gen0reductionCount,
        uint32_t reason,
        uint32_t globalMechanisms,
        uint32_t pauseMode,
        uint32_t memoryPressure);

};

extern GCToCLREventSink g_gcToClrEventSink;

#endif // __GCTOCLREVENTSINK_H__

