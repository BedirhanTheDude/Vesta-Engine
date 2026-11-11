#pragma once

#include <property/Property.h>

class Archive;

namespace PropertySerializer {
    void serialize(const PropertyHolder& holder, Archive& arch);
    void deserialize(PropertyHolder& holder, const Archive& arch);
}