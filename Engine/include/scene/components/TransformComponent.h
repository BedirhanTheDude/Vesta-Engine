#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <scene/components/Component.h>

#include <set>
#include <vector>
#include <functional>

class Archive;

class TransformComponent : public Component {
public:
	TransformComponent();
	~TransformComponent();

	glm::mat4 getMatrix() const;

	void translate(const glm::vec3& delta);
	void setPosition(const glm::vec3& pos);

	void setRotation(const glm::vec3& eulerAngles);
	void setRotation(const glm::quat& quat);
	void rotate(const glm::vec3& eulerDelta);
	void rotateAroundAxis(const glm::vec3& axis, float angle);

	void setScale(const glm::vec3& scale);
	void scaleBy(const glm::vec3& factor);

	void serialize(Archive& arch) const override;
	void deserialize(const Archive& arch) override;

	void setParent(TransformComponent* newParent);
	void removeChild(TransformComponent* child);

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

	TransformComponent* getParent() const { return parent; }
	const std::vector<TransformComponent*>& getChildren() const { return children; }
	const std::set<unsigned int>& getChildrenIDs() const { return childrenIDs; }

	std::function<bool()> onBeforeReparent = nullptr;

	mutable bool physicsDirty = true;
private:
	void invalidate();

	TransformComponent* parent = nullptr;
	std::vector<TransformComponent*> children;
	std::set<unsigned int> childrenIDs;

	mutable bool isMatrixValid = false;
	mutable bool worldEulerValid = false;
	mutable glm::mat4 modelMatrix = glm::mat4(1.0f);
	mutable glm::vec3 cachedWorldEuler = glm::vec3(0.0f);

	glm::vec3 position{ 0.0f, 0.0f, 0.0f };
	glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
	glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

	mutable bool eulerDirty = true;
	mutable glm::vec3 cachedEuler{ 0.0f };
};