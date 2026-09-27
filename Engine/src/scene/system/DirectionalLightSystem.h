#pragma once

#include <glm/glm.hpp>

#include <scene/components/DirectionalLight.h>
#include <scene/components/Transform.h>

class Archive;

namespace DirectionalLightSystem {

	glm::vec3 getDirection(const Transform& transform);
	struct ShadowFrustum {
		float size;      // width and height of the covered area, world units
		float nearPlane; // depth range measured from the light
		float farPlane;
	};

	ShadowFrustum getShadowFrustum(const DirectionalLight& light);

	// The shadow box follows the camera: the light sits shadowDistance behind the box center, looks at it and
	// covers a size x size area over the depth range [near, far].
	// NOTE: I am not sure if I like the current shadow map boundary
	glm::mat4 getLightSpaceMatrix(const DirectionalLight& light, const Transform& transform,
		const glm::vec3& camPos, const glm::vec3& camForward = glm::vec3(0.0f));

	void serialize(const DirectionalLight& light, Archive& arch);
	void deserialize(DirectionalLight& light, const Archive& arch);
}
