#include <controller/TransformController.h>

#include <controller/EntityController.h>

#include <scene/Entity.h>
#include <scene/components/TransformComponent.h>

glm::vec3 TransformController::getPosition(unsigned int entityID) {
	Entity entity = EntityController::resolveEntity(entityID);
	if (!entity.isValid()) return glm::vec3(0.0f);

	return entity.getTransform().getPosition();
}

glm::vec3 TransformController::getRotation(unsigned int entityID) {
	Entity entity = EntityController::resolveEntity(entityID);
	if (!entity.isValid()) return glm::vec3(0.0f);

	return entity.getTransform().getEulerRotation();
}

glm::vec3 TransformController::getScale(unsigned int entityID) {
	Entity entity = EntityController::resolveEntity(entityID);
	if (!entity.isValid()) return glm::vec3(1.0f);

	return entity.getTransform().getScale();
}

void TransformController::setPosition(unsigned int entityID, const glm::vec3& pos) {
	Entity entity = EntityController::resolveEntity(entityID);
	if (!entity.isValid()) return;

	entity.getTransform().setPosition(pos);
}

void TransformController::setRotation(unsigned int entityID, const glm::vec3& rot) {
	Entity entity = EntityController::resolveEntity(entityID);
	if (!entity.isValid()) return;

	entity.getTransform().setRotation(rot);
}

void TransformController::setScale(unsigned int entityID, const glm::vec3& scale) {
	Entity entity = EntityController::resolveEntity(entityID);
	if (!entity.isValid()) return;

	entity.getTransform().setScale(scale);
}

glm::mat4 TransformController::getWorldMatrix(unsigned int entityID) {
	Entity entity = EntityController::resolveEntity(entityID);
	if (!entity.isValid()) return glm::mat4(1.0f);

	return entity.getTransform().getMatrix();
}

glm::mat4 TransformController::getParentWorldMatrix(unsigned int entityID) {
	Entity entity = EntityController::resolveEntity(entityID);
	if (!entity.isValid()) return glm::mat4(1.0f);

	Entity parent = entity.getTransform().getParent();
	if (parent.isValid())
		return parent.getTransform().getMatrix();

	return glm::mat4(1.0f);
}
