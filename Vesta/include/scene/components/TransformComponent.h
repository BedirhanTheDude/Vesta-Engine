#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <scene/components/Component.h>

#include <cstddef>
#include <vector>

class TransformComponent : public Component {
public:
	explicit TransformComponent(const Entity& entity) : Component(entity) {}

	glm::mat4 getMatrix() const;

	void translate(const glm::vec3& delta);
	void setPosition(const glm::vec3& pos);

	void setRotation(const glm::vec3& eulerAngles);
	void setRotation(const glm::quat& quat);
	void rotate(const glm::vec3& eulerDelta);
	void rotateX(float angleDegrees);
	void rotateY(float angleDegrees);
	void rotateZ(float angleDegrees);
	void rotateAroundAxis(const glm::vec3& axis, float angle);
	void rotateAroundLocalAxis(const glm::vec3& localAxis, float angle);

	void setScale(const glm::vec3& scale);
	void scaleBy(const glm::vec3& factor);

	// Keeps the world transform. Reparenting onto itself, onto one of its own descendants or
	// onto an entity of another scene is ignored, so is reparenting an entity that has a
	// RigidBodyComponent. The invalid Entity() unparents.
	void setParent(const Entity& newParent);

	glm::mat3 getRotationMatrix() const;

	glm::vec3 forward() const;
	glm::vec3 right() const;
	glm::vec3 up() const;

	glm::vec3 getPosition() const;
	glm::vec3 getEulerRotation() const;
	glm::quat getRotationQuat() const;
	glm::vec3 getRawEulerRotation() const;
	glm::vec3 getScale() const;
	glm::vec3 getWorldPosition() const;
	glm::vec3 getWorldEulerAngles() const;
	glm::quat getWorldRotationQuat() const;
	glm::vec3 getWorldScale() const;

	// Transform serialization/deserialization is tied to entity serialization/deserialization since it always exists
	void serialize(Archive& arch) const override {};
	void deserialize(const Archive& arch) override {};

	// The invalid Entity() if there is no parent. A parent that was removed reports isAlive() == false.
	Entity getParent() const;

	std::size_t getChildCount() const;
	Entity getChild(std::size_t index) const;

	std::vector<Entity> getChildren() const;
};
