// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#include "common.h"
#include "gctoclreventsink.h"
#include "eventtrace.h"

GCToCLREventSink g_gcToClrEventSink;

void GCToCLREventSink::FireDynamicEvent(const char* eventName, void* payload, uint32_t payloadSize)
{
    LIMITED_METHOD_CONTRACT;

    const size_t EventNameMaxSize = 255;

    WCHAR wideEventName[EventNameMaxSize];
    if (MultiByteToWideChar(CP_ACP, 0, eventName, -1, wideEventName, EventNameMaxSize) == 0)
    {
        return;
    }

    FireEtwGCDynamicEvent(wideEventName, payloadSize, (const BYTE*)payload, GetClrInstanceId());
}

void GCToCLREventSink::FireGCPerHeapHistory_V3(void *freeListAllocated,
                                               void *freeListRejected,
                                               void *endOfSegAllocated,
                                               void *condemnedAllocated,
                                               void *pinnedAllocated,
                                               void *pinnedAllocatedAdvance,
                                               uint32_t runningFreeListEfficiency,
                                               uint32_t condemnReasons0,
                                               uint32_t condemnReasons1,
                                               uint32_t compactMechanisms,
                                               uint32_t expandMechanisms,
                                               uint32_t heapIndex,
                                               void *extraGen0Commit,
                                               uint32_t count,
                                               uint32_t valuesLen,
                                               void *values)
{    
    FireEtwGCPerHeapHistory_V3(GetClrInstanceId(),
                               freeListAllocated,
                               freeListRejected,
                               endOfSegAllocated,
                               condemnedAllocated,
                               pinnedAllocated,
                               pinnedAllocatedAdvance,
                               runningFreeListEfficiency,
                               condemnReasons0,
                               condemnReasons1,
                               compactMechanisms,
                               expandMechanisms,
                               heapIndex,
                               extraGen0Commit,
                               count,
                               valuesLen,
                               values);
}



void GCToCLREventSink::FireBGCBegin()
{
    FireEtwBGCBegin(GetClrInstanceId());
}

void GCToCLREventSink::FireBGC1stNonConEnd()
{
    FireEtwBGC1stNonConEnd(GetClrInstanceId());
}

void GCToCLREventSink::FireBGC1stConEnd()
{
    FireEtwBGC1stConEnd(GetClrInstanceId());
}

void GCToCLREventSink::FireBGC2ndNonConBegin()
{
    FireEtwBGC2ndNonConBegin(GetClrInstanceId());
}

void GCToCLREventSink::FireBGC2ndNonConEnd()
{
    FireEtwBGC2ndNonConEnd(GetClrInstanceId());
}

void GCToCLREventSink::FireBGC2ndConBegin()
{
    FireEtwBGC2ndConBegin(GetClrInstanceId());
}

void GCToCLREventSink::FireBGC2ndConEnd()
{
    FireEtwBGC2ndConEnd(GetClrInstanceId());
}

void GCToCLREventSink::FireBGCDrainMark(uint64_t objects)
{
    FireEtwBGCDrainMark(objects, GetClrInstanceId());
}

void GCToCLREventSink::FireBGCRevisit(uint64_t pages, uint64_t objects, uint32_t isLarge)
{
    FireEtwBGCRevisit(pages, objects, isLarge, GetClrInstanceId());
}

void GCToCLREventSink::FireBGCOverflow(uint64_t min, uint64_t max, uint64_t objects, uint32_t isLarge)
{
    FireEtwBGCOverflow(min, max, objects, isLarge, GetClrInstanceId());
}

void GCToCLREventSink::FireBGCAllocWaitBegin(uint32_t reason)
{
    FireEtwBGCAllocWaitBegin(reason, GetClrInstanceId());
}

void GCToCLREventSink::FireBGCAllocWaitEnd(uint32_t reason)
{
    FireEtwBGCAllocWaitEnd(reason, GetClrInstanceId());
}

void GCToCLREventSink::FireGCFullNotify_V1(uint32_t genNumber, uint32_t isAlloc)
{
    FireEtwGCFullNotify_V1(genNumber, isAlloc, GetClrInstanceId());
}

void GCToCLREventSink::FireSetGCHandle(void *handleID, void *objectID, uint32_t kind, uint32_t generation, uint64_t appDomainID)
{
    FireEtwSetGCHandle(handleID, objectID, kind, generation, appDomainID, GetClrInstanceId());
}

void GCToCLREventSink::FirePrvSetGCHandle(void *handleID, void *objectID, uint32_t kind, uint32_t generation, uint64_t appDomainID)
{
    FireEtwPrvSetGCHandle(handleID, objectID, kind, generation, appDomainID, GetClrInstanceId());
}

void GCToCLREventSink::FireDestroyGCHandle(void *handleID)
{
    FireEtwDestroyGCHandle(handleID, GetClrInstanceId());
}

void GCToCLREventSink::FirePrvDestroyGCHandle(void *handleID)
{
    FireEtwPrvDestroyGCHandle(handleID, GetClrInstanceId());
}

void GCToCLREventSink::FireGCStart_V2(uint32_t count, uint32_t depth, uint32_t reason, uint32_t type)
{
    LIMITED_METHOD_CONTRACT;

    ETW::GCLog::ETW_GC_INFO gcStartInfo;
    gcStartInfo.GCStart.Count = count;
    gcStartInfo.GCStart.Depth = depth;
    gcStartInfo.GCStart.Reason = static_cast<ETW::GCLog::ETW_GC_INFO::GC_REASON>(reason);
    gcStartInfo.GCStart.Type = static_cast<ETW::GCLog::ETW_GC_INFO::GC_TYPE>(type);
    ETW::GCLog::FireGcStart(&gcStartInfo);
}

void GCToCLREventSink::FireGCGenerationRange(uint8_t generation, void* rangeStart, uint64_t rangeUsedLength, uint64_t rangeReservedLength)
{
    LIMITED_METHOD_CONTRACT;

    FireEtwGCGenerationRange(generation, rangeStart, rangeUsedLength, rangeReservedLength, GetClrInstanceId());
}

void GCToCLREventSink::FireGCEnd_V1(uint32_t count, uint32_t depth)
{
    LIMITED_METHOD_CONTRACT;

    FireEtwGCEnd_V1(count, depth, GetClrInstanceId());
}

void GCToCLREventSink::FireGCHeapStats_V1(
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
        uint32_t gcHandleCount)
{
    LIMITED_METHOD_CONTRACT;

    FireEtwGCHeapStats_V1(generationSize0, totalPromotedSize0, generationSize1, totalPromotedSize1,
                          generationSize2, totalPromotedSize2, generationSize3, totalPromotedSize3,
                          finalizationPromotedSize, finalizationPromotedCount, pinnedObjectCount,
                          sinkBlockCount, gcHandleCount, GetClrInstanceId());
}

void GCToCLREventSink::FireGCCreateSegment_V1(void* address, size_t size, uint32_t type)
{
    LIMITED_METHOD_CONTRACT;

    FireEtwGCCreateSegment_V1((uint64_t)address, static_cast<uint64_t>(size), type, GetClrInstanceId());
}

void GCToCLREventSink::FireGCFreeSegment_V1(void* address)
{
    LIMITED_METHOD_CONTRACT;

    FireEtwGCFreeSegment_V1((uint64_t)address, GetClrInstanceId());
}

void GCToCLREventSink::FireGCCreateConcurrentThread_V1()
{
    LIMITED_METHOD_CONTRACT;

    FireEtwGCCreateConcurrentThread_V1(GetClrInstanceId());
}

void GCToCLREventSink::FireGCTerminateConcurrentThread_V1()
{
    LIMITED_METHOD_CONTRACT;

    FireEtwGCTerminateConcurrentThread_V1(GetClrInstanceId());
}

