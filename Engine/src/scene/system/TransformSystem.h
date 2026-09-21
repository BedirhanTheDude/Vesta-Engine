#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <scene/components/Transform.h>

class Archive;
class Scene;

namespace ECS {
	struct EntityHandle;
}

namespace TransformSystem {

	glm::mat4 getMatrix(const Transform& transform, Scene* scene = nullptr);

	void translate(Transform& transform, const glm::vec3& delta, Scene* scene = nullptr);
	void setPosition(Transform& transform, const glm::vec3& pos, Scene* scene = nullptr);

	void setRotation(Transform& transform, const glm::vec3& eulerDegrees, Scene* scene = nullptr);
	void setRotation(Transform& transform, const glm::quat& quat, Scene* scene = nullptr);
	void rotate(Transform& transform, const glm::vec3& eulerDeltaDegrees, Scene* scene = nullptr);
	void rotateAroundAxis(Transform& transform, const glm::vec3& axis, float angleDegrees, Scene* scene = nullptr);

	void setScale(Transform& transform, const glm::vec3& scale, Scene* scene = nullptr);
	void scaleBy(Transform& transform, const glm::vec3& factor, Scene* scene = nullptr);

	void serialize(const Transform& transform, Archive& arch);
	void deserialize(Transform& transform, const Archive& arch);

	void setParent(Transform& transform, const ECS::EntityHandle& entityHandle, 
		const ECS::EntityHandle& parentHandle, Scene* scene = nullptr);
	void removeChild(Transform& transform, const ECS::EntityHandle& childHandle);

	glm::mat3 getRotationMatrix(const Transform& transform);

	glm::vec3 forward(const Transform& transform);
	glm::vec3 right(const Transform& transform);
	glm::vec3 up(const Transform& transform);

	glm::vec3 getPosition(const Transform& transform);
	glm::vec3 getEulerRotation(const Transform& transform);
	glm::quat getRotationQuat(const Transform& transform);
	glm::vec3 getRawEulerRotation(const Transform& transform);
	glm::vec3 getScale(const Transform& transform);
	glm::vec3 getWorldPosition(const Transform& transform, Scene* scene = nullptr);
	glm::vec3 getWorldEulerAngles(const Transform& transform, Scene* scene = nullptr);
	glm::quat getWorldRotationQuat(const Transform& transform, Scene* scene = nullptr);
	glm::vec3 getWorldScale(const Transform& transform, Scene* scene = nullptr);

	// Try to get entity's parent, returns false if there is NO parent
	bool tryGetParent(const Transform& transform, ECS::EntityHandle& parentHandle);
	const std::vector<ECS::EntityHandle>& getChildren(const Transform& transform);
}