#include <controller/ComponentController.h>

#include <controller/EntityController.h>

#include <scene/Entity.h>
#include <scene/components/Component.h>
#include <scene/components/BehaviourComponent.h>
#include <scene/components/ComponentFactory.h>
#include <persistance/Archive.h>
#include <property/ComponentReflection.h>

#include <cstdio>
#include <cstdint>
#include <exception>
#include <unordered_map>

namespace {
	struct CachedReflection {
		Entity entity;
		ReflectedComponent reflected;
	};

	// one entry per (entity, component) pair,
	// rebuilt only when isCurrent() says the fingerprints are stale
	std::unordered_map<uint64_t, CachedReflection> reflectionCache;

	// dead entities and removed components leave entries behind, drop them once there are many
	void pruneReflectionCache() {
		for (auto it = reflectionCache.begin(); it != reflectionCache.end();) {
			if (!it->second.entity.isAlive()) it = reflectionCache.erase(it);
			else ++it;
		}
	}

	ReflectedComponent* getReflection(const Entity& entity, unsigned int UID) {
		uint64_t key = (static_cast<uint64_t>(entity.getID()) << 32) | UID;

		auto it = reflectionCache.find(key);
		if (it != reflectionCache.end() && it->second.entity == entity &&
			ComponentReflection::isCurrent(entity, UID, it->second.reflected))
			return &it->second.reflected;

		if (reflectionCache.size() > 256)
			pruneReflectionCache();

		CachedReflection& slot = reflectionCache[key];
		slot.entity = entity;

		if (!ComponentReflection::build(entity, UID, slot.reflected)) {
			reflectionCache.erase(key);
			return nullptr;
		}

		return &slot.reflected;
	}
}

void ComponentController::createAndBindComponent(const std::string& componentName, unsigned int entityID) {
	Entity entity = EntityController::resolveEntity(entityID);
	if (!entity.isValid()) return;

	// creating throws if the entity already has the component (dropping a script that is already on it)
	// or the component rejects attaching, neither should take the editor down
	// NOTE: Maybe not throw but kindly remind the user that they are doing something wrong?
	try {
		ComponentFactory::create(componentName, entity, Archive());
	}
	catch (const std::exception& e) {
		printf("Could not add %s: %s as the entity already has this component\n", componentName.c_str(), e.what());
	}
}

void ComponentController::removeComponent(unsigned int entityID, unsigned int componentUID) {
	Entity entity = EntityController::resolveEntity(entityID);
	if (!entity.isValid()) return;

	entity.removeComponent(componentUID);
}

bool ComponentController::entityHasComponent(unsigned int entityID, const std::string& componentName) {
	Entity entity = EntityController::resolveEntity(entityID);
	if (!entity.isValid()) return false;

	return entity.hasComponent(componentNameToUID(componentName));
}

const std::vector<std::string>& ComponentController::getAvailableComponentNames() {
	const std::vector<std::string>& names = ComponentFactory::getBuiltInNames();

	return names;
}

const std::vector<std::string>& ComponentController::getAvailableScriptNames() {
	return ComponentFactory::getScriptNames();
}

std::vector<unsigned int> ComponentController::getEntityComponentUIDs(unsigned int entityID) {
	Entity entity = EntityController::resolveEntity(entityID);
	if (!entity.isValid()) return std::vector<unsigned int>();

	return ComponentFactory::getEntityComponentUIDs(entity);
}

const std::map<unsigned int, Property>& ComponentController::getComponentProperties(unsigned int entityID, unsigned int componentUID, unsigned int groupID) {
	static const std::map<unsigned int, Property> empty;

	const std::vector<PropertyGroup>& groups = getComponentPropertyGroups(entityID, componentUID);
	if (groupID >= groups.size()) return empty;

	return groups[groupID].properties;
}

const std::vector<PropertyGroup>& ComponentController::getComponentPropertyGroups(unsigned int entityID, unsigned int componentUID) {
	static const std::vector<PropertyGroup> empty;

	Entity entity = EntityController::resolveEntity(entityID);
	if (!entity.isValid()) return empty;

	if (ReflectedComponent* reflected = getReflection(entity, componentUID))
		return reflected->getPropertyGroups();

	// scripts carry their own properties, their objects never move
	if (BehaviourComponent* behaviour = entity.getBehaviour(componentUID))
		return behaviour->getPropertyGroups();

	return empty;
}

const std::map<unsigned int, Payload>& ComponentController::getComponentPayloads(unsigned int entityID, unsigned int componentUID, unsigned int groupID) {
	static const std::map<unsigned int, Payload> empty;

	const std::vector<PayloadGroup>& groups = getComponentPayloadGroups(entityID, componentUID);
	if (groupID >= groups.size()) return empty;

	return groups[groupID].payloads;
}

const std::vector<PayloadGroup>& ComponentController::getComponentPayloadGroups(unsigned int entityID, unsigned int componentUID) {
	static const std::vector<PayloadGroup> empty;

	Entity entity = EntityController::resolveEntity(entityID);
	if (!entity.isValid()) return empty;

	if (ReflectedComponent* reflected = getReflection(entity, componentUID))
		return reflected->getPayloadGroups();

	if (BehaviourComponent* behaviour = entity.getBehaviour(componentUID))
		return behaviour->getPayloadGroups();

	return empty;
}

const std::vector<CallbackPropertyGroup>& ComponentController::getComponentCallbackPropertyGroups(unsigned int entityID, unsigned int componentUID) {
	static const std::vector<CallbackPropertyGroup> empty;

	Entity entity = EntityController::resolveEntity(entityID);
	if (!entity.isValid()) return empty;

	if (ReflectedComponent* reflected = getReflection(entity, componentUID))
		return reflected->getCallbackPropertyGroups();

	if (BehaviourComponent* behaviour = entity.getBehaviour(componentUID))
		return behaviour->getCallbackPropertyGroups();

	return empty;
}

std::string ComponentController::componentUIDToString(unsigned int componentUID) {
	return componentTypeUIDToString(componentUID);
}
