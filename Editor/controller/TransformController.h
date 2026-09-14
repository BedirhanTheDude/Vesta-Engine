#pragma once

#include <glm/glm.hpp>

class TransformController {
public:
	static glm::vec3 getPosition(unsigned int entityID);
	static glm::vec3 getRotation(unsigned int entityID);
	static glm::vec3 getScale(unsigned int entityID);

	static void setPosition(unsigned int entityID, const glm::vec3& pos);
	static void setRotation(unsigned int entityID, const glm::vec3& rot);
	static void setScale(unsigned int entityID, const glm::vec3& scale);

	// world-space matrices for gizmo manipulation; parent is identity when the entity is a root
	static glm::mat4 getWorldMatrix(unsigned int entityID);
	static glm::mat4 getParentWorldMatrix(unsigned int entityID);
};