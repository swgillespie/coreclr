// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#include "gcconfig.h"

// GCConfig* g_pConfig;

// temp hacks
namespace std {
    char *getenv(const char*);
    int atoi(const char*);
    long atol(const char*);
    long long atoll(const char*);
}

#define ENV_VAR_PREFIX "CLRGC_"

class GCConfig::Impl {
#define GC_CONFIG(type, name)                                          \
private:                                                               \
    bool name##_present;                                               \
    type name##_value;                                                 \
public:                                                                \
    type Get##name() {                                                 \
        if (name##_present) return name##_value;                       \
        name##_value = GetConfig<type>(#name, ENV_VAR_PREFIX #name);   \
        name##_present = true;                                         \
        return name##_value;                                           \
    }

#include "gcconfig.def"

private:
    template<typename T>
    T GetConfig(const char *key, const char *env_var)
    {
        // TODO(local-gc) Once we stop using PAL headers, we can use
        // std::getenv to read from environment variables
        UNREFERENCED_PARAMETER(env_var);

        // ask the EE to retreive this particular configuration value
        // for us
        void *value = GCToEEInterface::GetConfigValue(key);
        if (!value) {
            return T();
        }

        return (T)(value);
    }
};

GCConfig::GCConfig()
{
    // the impl object is global and shared amongst instances of GCConfig
    // (since there should only be one GCConfig anyway).
    static GCConfig::Impl impl;
    _impl = &impl;
}

GCConfig::~GCConfig() {}

#define GC_CONFIG(type, name) \
type GCConfig::Get##name() const { \
    return _impl->Get##name();     \
}

#include "gcconfig.def"
