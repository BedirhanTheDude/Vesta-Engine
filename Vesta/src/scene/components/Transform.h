#pragma once

#include <ecs/EntityHandle.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>

// TODO: Negative scales are currently not supported due to reparenting issues 
struct Transform {
	std::vector<ECS::EntityHandle> children;
	ECS::EntityHandle parent = ECS::INVALID_ENTITY_HANDLE;
	
	glm::vec3 position	{ 0.0f, 0.0f, 0.0f };
	glm::quat rotation	{ 1.0f, 0.0f, 0.0f, 0.0f };
	glm::vec3 scale		{ 1.0f, 1.0f, 1.0f };

	mutable glm::mat4 modelMatrix	   = glm::mat4(1.0f);
	mutable glm::vec3 cachedWorldEuler = glm::vec3(0.0f);
	mutable glm::vec3 cachedEuler	   = glm::vec3(0.0f);

	mutable bool isMatrixValid = false;
	mutable bool worldEulerValid = false;
	mutable bool eulerDirty = true;
	mutable bool physicsDirty = true;
};