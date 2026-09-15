#pragma once

#include <glm/glm.hpp>

namespace TransformController {
	glm::vec3 getPosition(unsigned int entityID);
	glm::vec3 getRotation(unsigned int entityID);
	glm::vec3 getScale(unsigned int entityID);

	void setPosition(unsigned int entityID, const glm::vec3& pos);
	void setRotation(unsigned int entityID, const glm::vec3& rot);
	void setScale(unsigned int entityID, const glm::vec3& scale);

	// world-space matrices for gizmo manipulation; parent is identity when the entity is a root
	glm::mat4 getWorldMatrix(unsigned int entityID);
	glm::mat4 getParentWorldMatrix(unsigned int entityID);
};