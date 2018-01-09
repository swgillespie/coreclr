// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef __GCEVENT_H__
#define __GCEVENT_H__

#include "common.h"
#include "gcenv.h"
#include "gcevent_serializers.h"

namespace gc_event
{

class GCEventDescriptorBase
{
public:
    GCEventDescriptorBase(const char* name, GCEventLevel level, int keyword)
        : m_eventName(name), m_level(level), m_keyword(keyword)
    {}

    GCEventLevel Level() const { return m_level; }

    int Keyword() const { return m_keyword; }

    const char* EventName() const
    {
        return m_eventName;
    }

private:
    const char* m_eventName;
    GCEventLevel m_level;
    int m_keyword;
};

template<class... EventArgument>
class GCEventDescriptor : public GCEventDescriptorBase
{
public:
    GCEventDescriptor(const char* name, int level, int keyword)
        : GCEventDescriptorBase(name, level, keyword)
    {}

    void FireEvent(EventArgument... arguments)
    {
        size_t size = SerializedSize(arguments...);
        uint8_t* buf = new (nothrow) uint8_t[size];
        if (!buf)
        {
            // best effort - if we're OOM, don't bother with the event.
            return;
        }


        Serialize(&buf, arguments...);
        // GCToEEInterface::FireDynamicEvent(m_eventName, buf, size);
    }
};

} // namespace gc_event

#endif // __GCEVENT_H__
