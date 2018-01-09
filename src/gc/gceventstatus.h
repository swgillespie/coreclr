// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef __GCEVENTSTATUS_H__
#define __GCEVENTSTATUS_H__

#include "common.h"
#include "gcenv.h"
#include "gc.h"

enum GCEventProvider
{
    GCEventProvider_Default = 0,
    GCEventProvider_Private = 1
};

class GCEventStatus
{
private:
    static Volatile<GCEventLevel> enabledLevels[2];
    static Volatile<GCEventKeyword> enabledKeywords[2];

public:
    static bool IsEnabled(GCEventProvider provider, GCEventKeyword keyword, GCEventLevel level)
    {
        assert(level >= GCEventLevel_None && level < GCEventLevel_Max);

        size_t index = static_cast<size_t>(provider);
        return (enabledLevels[index] >= level) && (enabledKeywords[index] & keyword);
    }

    static void Enable(GCEventProvider provider, GCEventKeyword keyword, GCEventLevel level)
    {
        assert(level >= GCEventLevel_None && level < GCEventLevel_Max);

        size_t index = static_cast<size_t>(provider);
        enabledLevels[index] = level;
        enabledKeywords[index] = enabledKeywords[index] | keyword;
    }

    static void Disable(GCEventProvider provider, GCEventKeyword keyword, GCEventLevel level)
    {
        assert(level >= GCEventLevel_None && level < GCEventLevel_Max);

        size_t index = static_cast<size_t>(provider);
        enabledLevels[index] = GCEventLevel_None;
        enabledKeywords[index] = enabledKeywords[index] & ~keyword;
    }
};

#endif // __GCEVENTSTATUS_H__
