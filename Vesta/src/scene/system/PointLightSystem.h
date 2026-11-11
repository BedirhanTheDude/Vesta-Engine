#pragma once

#include <scene/components/PointLight.h>

class Archive;

namespace PointLightSystem {

	void serialize(const PointLight& light, Archive& arch);
	void deserialize(PointLight& light, const Archive& arch);
}
