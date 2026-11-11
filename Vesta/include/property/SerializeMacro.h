#pragma once

#include <property/PropertyTraits.h>

#define SERIALIZE(type, name, ...) \
    type name = __VA_ARGS__; \
    bool _reg_##name = [this]() { \
        registerProperty(#name, PropertyTypeOf<type>::value, &name); \
        return true; \
    }()