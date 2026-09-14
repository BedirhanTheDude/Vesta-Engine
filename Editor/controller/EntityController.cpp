#include <controller/EntityController.h>

#include <scene/Scene.h>
#include <scene/Entity.h>
#include <scene/components/TransformComponent.h>

#include <core/Application.h>

#include <cstdint>

int64_t EntityController::selectedEntityID = -1;

const std::set<unsigned int>& EntityController::getAllEntityIDs() {
	Scene* scene = Application::getCurrentScene();
	return scene->getEntityIDs();
}

const std::set<unsigned int>& EntityController::getEntityChildIDs(unsigned int parentID) {
	static const std::set<unsigned int> empty;

	Entity* parent = resolveEntity(parentID);

	if (!parent) return empty;

	return parent->transform.getChildrenIDs();
}

const std::map<unsigned int, unsigned int>& EntityController::getComponentIdxMap(unsigned int entityID) {
	static const std::map<unsigned int, unsigned int> empty;

	Entity* entity = resolveEntity(entityID);

	if (!entity) return empty;

	return entity->getComponentIdxMap();
}

void EntityController::setSelectedEntityID(unsigned int ID) {
	Entity* entity = resolveEntity(ID);

	if (entity) 
		selectedEntityID = (int64_t)ID;
	// else add console log
}

void EntityController::setEntityParent(unsigned int childID, unsigned int parentID) {
	Entity* childEntity = resolveEntity(childID);
	Entity* parentEntity = resolveEntity(parentID);

	if (childEntity && parentEntity)
		childEntity->transform.setParent(&parentEntity->transform);
}

void EntityController::unparentEntity(unsigned int ID) {
	Entity* entity = resolveEntity(ID);

	if (entity)
		entity->transform.setParent(nullptr);
}

void EntityController::clearSelectedEntityID() {
	selectedEntityID = -1;
}

void EntityController::renameEntity(unsigned int ID, const char* name) {
	Entity* entity = resolveEntity(ID);

	if (entity)
		entity->setName(std::string(name));
}

void EntityController::removeEntity(unsigned int ID) {
	Scene* scene = Application::getCurrentScene();

	scene->removeEntity(ID);
}

Entity* EntityController::getSelectedEntity() {
	if (selectedEntityID < 0) return nullptr;
	return resolveEntity((unsigned int)selectedEntityID);
}

void EntityController::setSelectedEntity(Entity* entity) {
	selectedEntityID = entity ? (int64_t)entity->getID() : -1;
}

int64_t EntityController::getSelectedEntityID() {
	return selectedEntityID;
}

std::string EntityController::getEntityName(unsigned int ID) {
	Entity* entity = resolveEntity(ID);

	if (entity)
		return entity->getName();
	else return std::string("");
}

bool EntityController::entityHasParent(unsigned int ID) {
	Entity* entity = resolveEntity(ID);

	if (entity && entity->transform.getParent())
		return true;
	else
		return false;
}

bool EntityController::wouldCreateCycle(unsigned int childID, unsigned int newParentID) {
	Entity* dragged = resolveEntity(childID);
	Entity* target = resolveEntity(newParentID);

	if (!dragged || !target) return true; // fail safe, refuse if either side is invalid
	if (dragged == target) return true; // can't parent to self

	TransformComponent* check = &target->transform;
	while (check) {
		if (check == &dragged->transform) return true;
		check = check->getParent();
	}
	return false;
}

Entity* EntityController::resolveEntity(unsigned int ID) {
	Scene* scene = Application::getCurrentScene();
	return scene->findEntity(ID);
}