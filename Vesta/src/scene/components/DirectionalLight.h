#pragma once

#include <glm/glm.hpp>

struct DirectionalLight {
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

	float ambientStrength = 0.2f;
	float diffuseStrength = 1.0f;
	float specularStrength = 0.5f;

	float shadowDistance = 30.0f;
	float shadowOrthoSize = 40.0f;
	float shadowNear = 0.1f;
	float shadowFar = 200.0f;
};
