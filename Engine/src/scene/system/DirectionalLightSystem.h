#pragma once

#include <glm/glm.hpp>

#include <scene/components/DirectionalLight.h>
#include <scene/components/Transform.h>

class Archive;

namespace DirectionalLightSystem {

	void serialize(const DirectionalLight& light, Archive& arch);
	void deserialize(DirectionalLight& light, const Archive& arch);
}
