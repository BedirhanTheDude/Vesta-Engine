#include <controller/EntityController.h>

#include <core/Project.h>
#include <scene/Scene.h>
#include <scene/components/TransformComponent.h>

#include <core/Application.h>

#include <cstdint>

static Entity selectedEntity;

std::vector<unsigned int> EntityController::getAllEntityIDs() {
	std::vector<unsigned int> IDs;

	Scene* scene = Application::getCurrentScene();
	if (!scene) return IDs;

	for (const Entity& entity : scene->getEntities())
		IDs.push_back(entity.getID());

	return IDs;
}

std::vector<unsigned int> EntityController::getEntityChildIDs(unsigned int parentID) {
	std::vector<unsigned int> IDs;

	Entity parent = resolveEntity(parentID);
	if (!parent.isValid()) return IDs;

	for (const Entity& child : parent.getTransform().getChildren()) {
		if (child.isAlive())
			IDs.push_back(child.getID());
	}

	return IDs;
}

void EntityController::setSelectedEntityID(unsigned int ID) {
	Entity entity = resolveEntity(ID);

	if (entity.isValid())
		selectedEntity = entity;
	// else add console log
}

void EntityController::setEntityParent(unsigned int childID, unsigned int parentID) {
	Entity child = resolveEntity(childID);
	Entity parent = resolveEntity(parentID);

	if (child.isValid() && parent.isValid())
		child.getTransform().setParent(parent);
}

void EntityController::unparentEntity(unsigned int ID) {
	Entity entity = resolveEntity(ID);

	if (entity.isValid())
		entity.getTransform().setParent(Entity());
}

void EntityController::clearSelectedEntityID() {
	selectedEntity = Entity();
}

void EntityController::renameEntity(unsigned int ID, const char* name) {
	Entity entity = resolveEntity(ID);

	if (entity.isValid() && name)
		entity.setName(std::string(name));
}

void EntityController::removeEntity(unsigned int ID) {
	Scene* scene = Application::getCurrentScene();
	if (!scene) return;

	scene->removeEntity(ID);
}

bool EntityController::canPasteEntity() {
	return Project::hasCopiedEntity();
}

bool EntityController::copyEntity(unsigned int ID) {
	return Project::copyEntity(static_cast<uint32_t>(ID));
}

void EntityController::pasteEntity() {
	Project::pasteEntity();
}

Entity EntityController::getSelectedEntity() {
	// removed since it was selected, or the scene was reloaded
	if (!selectedEntity.isAlive())
		selectedEntity = Entity();

	return selectedEntity;
}

void EntityController::setSelectedEntity(const Entity& entity) {
	selectedEntity = entity.isAlive() ? entity : Entity();
}

uint32_t EntityController::getSelectedEntityID() {
	Entity selected = getSelectedEntity();

	return selected.isValid() ? selected.getID() : UINT32_MAX;
}

std::string EntityController::getEntityName(unsigned int ID) {
	Entity entity = resolveEntity(ID);

	if (entity.isValid())
		return entity.getName();
	else return std::string("");
}

bool EntityController::isEntityAlive(const Entity& entity) {
	return entity.isAlive();
}

bool EntityController::entityHasParent(unsigned int ID) {
	Entity entity = resolveEntity(ID);

	return entity.isValid() && entity.getTransform().getParent().isValid();
}

bool EntityController::wouldCreateCycle(unsigned int childID, unsigned int newParentID) {
	Entity dragged = resolveEntity(childID);
	Entity target = resolveEntity(newParentID);

	if (!dragged.isValid() || !target.isValid()) return true; // fail safe, refuse if either side is invalid
	if (dragged == target) return true; // can't parent to self

	// walk up from the new parent, meeting the dragged entity means it would become its own ancestor
	for (Entity check = target; check.isValid(); check = check.getTransform().getParent()) {
		if (check == dragged) return true;
	}
	return false;
}

Entity EntityController::resolveEntity(unsigned int ID) {
	Scene* scene = Application::getCurrentScene();
	if (!scene) return Entity();

	return scene->findEntity(ID);
}
