#include <controller/ComponentController.h>

#include <controller/EntityController.h>

#include <scene/Entity.h>
#include <scene/components/Component.h>
#include <scene/components/ComponentFactory.h>
#include <persistance/Archive.h>

void ComponentController::createAndBindComponent(const std::string& componentName, unsigned int entityID) {
	Entity* entity = EntityController::resolveEntity(entityID);
	if (!entity) return;

	ComponentFactory::create(componentName, *entity, Archive());
}

void ComponentController::removeComponent(unsigned int entityID, unsigned int componentUID) {
	Entity* entity = EntityController::resolveEntity(entityID);
	if (!entity) return;

	entity->removeComponent(componentUID);
}

bool ComponentController::entityHasComponent(unsigned int entityID, const std::string& componentName) {
	Entity* entity = EntityController::resolveEntity(entityID);
	if (!entity) return false;

	return entity->getComponentIdxMap().count(componentNameToUID(componentName)) > 0;
}

std::vector<std::string> ComponentController::getAvailableComponentNames() {
	return ComponentFactory::getBuiltInNames();
}

const std::map<unsigned int, Property>& ComponentController::getComponentProperties(unsigned int entityID, unsigned int componentUID, unsigned int groupID) {
	static const std::map<unsigned int, Property> empty;

	Entity* entity = EntityController::resolveEntity(entityID);
	if (!entity) return empty;

	Component* comp = entity->getComponent(componentUID);
	if (comp)
		return comp->getProperties(groupID);
	else
		return empty;
}

const std::vector<PropertyGroup>& ComponentController::getComponentPropertyGroups(unsigned int entityID, unsigned int componentUID) {
	static const std::vector<PropertyGroup> empty;

	Entity* entity = EntityController::resolveEntity(entityID);
	if (!entity) return empty;

	Component* comp = entity->getComponent(componentUID);
	if (comp)
		return comp->getPropertyGroups();
	else
		return empty;
}

const std::map<unsigned int, Payload>& ComponentController::getComponentPayloads(unsigned int entityID, unsigned int componentUID, unsigned int groupID) {
	static const std::map<unsigned int, Payload> empty;

	Entity* entity = EntityController::resolveEntity(entityID);
	if (!entity) return empty;

	Component* comp = entity->getComponent(componentUID);
	if (comp)
		return comp->getPayloads(groupID);
	else
		return empty;
}

const std::vector<PayloadGroup>& ComponentController::getComponentPayloadGroups(unsigned int entityID, unsigned int componentUID) {
	static const std::vector<PayloadGroup> empty;

	Entity* entity = EntityController::resolveEntity(entityID);
	if (!entity) return empty;

	Component* comp = entity->getComponent(componentUID);
	if (comp)
		return comp->getPayloadGroups();
	else
		return empty;
}

const std::vector<CallbackPropertyGroup>& ComponentController::getComponentCallbackPropertyGroups(unsigned int entityID, unsigned int componentUID) {
	static const std::vector<CallbackPropertyGroup> empty;

	Entity* entity = EntityController::resolveEntity(entityID);
	if (!entity) return empty;

	Component* comp = entity->getComponent(componentUID);
	if (comp)
		return comp->getCallbackPropertyGroups();
	else
		return empty;
}

std::string ComponentController::componentUIDToString(unsigned int componentUID) {
	return componentTypeUIDToString(componentUID);
}
