#include <controller/TransformController.h>

#include <controller/EntityController.h>

#include <scene/Entity.h>
#include <scene/components/TransformComponent.h>

glm::vec3 TransformController::getPosition(unsigned int entityID) {
	Entity* entity = EntityController::resolveEntity(entityID);

	return entity->transform.getPosition();
}

glm::vec3 TransformController::getRotation(unsigned int entityID) {
	Entity* entity = EntityController::resolveEntity(entityID);

	return entity->transform.getEulerRotation();
}

glm::vec3 TransformController::getScale(unsigned int entityID) {
	Entity* entity = EntityController::resolveEntity(entityID);

	return entity->transform.getScale();
}

void TransformController::setPosition(unsigned int entityID, const glm::vec3& pos) {
	Entity* entity = EntityController::resolveEntity(entityID);

	entity->transform.setPosition(pos);
}

void TransformController::setRotation(unsigned int entityID, const glm::vec3& rot) {
	Entity* entity = EntityController::resolveEntity(entityID);

	entity->transform.setRotation(rot);
}

void TransformController::setScale(unsigned int entityID, const glm::vec3& scale) {
	Entity* entity = EntityController::resolveEntity(entityID);

	entity->transform.setScale(scale);
}

glm::mat4 TransformController::getWorldMatrix(unsigned int entityID) {
	Entity* entity = EntityController::resolveEntity(entityID);
	if (!entity) return glm::mat4(1.0f);

	return entity->transform.getMatrix();
}

glm::mat4 TransformController::getParentWorldMatrix(unsigned int entityID) {
	Entity* entity = EntityController::resolveEntity(entityID);
	if (!entity) return glm::mat4(1.0f);

	TransformComponent* parent = entity->transform.getParent();
	if (parent)
		return parent->getMatrix();

	return glm::mat4(1.0f);
}