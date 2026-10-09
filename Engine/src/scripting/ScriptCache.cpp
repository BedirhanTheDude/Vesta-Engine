#include "ScriptCache.h"

#include <persistance/Archive.h>
#include <ecs/ComponentPoolRegistry.h>

#include <scene/Scene.h>
#include <scene/Entity.h>
#include <scene/components/ComponentTypeUID.h>
#include <scene/components/ComponentFactory.h>

#include <cstdint>
#include <utility>
#include <string>

static Archive scriptsCacheArchive;
bool cacheSaved = false;

void ScriptCache::saveScriptState(const Scene& scene) {
	auto& behaviourPools = scene.getComponentPoolRegistry()->getBehaviourPools();
	scriptsCacheArchive = Archive();

	for (auto& [scriptUID, pool] : behaviourPools) {
		Archive scriptArchive;
		uint32_t index = 0;
		for (auto& [entityHandle, behaviour] : pool) {
			Archive entityArchive;
			entityArchive.set("entityId", entityHandle.entityId);
			entityArchive.set("generation", entityHandle.generation);

			behaviour->serialize(entityArchive);
			scriptArchive.set(std::to_string(index), std::move(entityArchive));
			++index;
		}
		scriptArchive.set("entityCount", index);
		std::string scriptName = componentTypeUIDToString(scriptUID);
		if (scriptName == "<unkown_component>") continue;
		scriptsCacheArchive.set(scriptName, std::move(scriptArchive));
	}

	cacheSaved = true;
}

void ScriptCache::loadScriptState(Scene& scene) {
	if (!cacheSaved) return;

	auto& behaviourPools = scene.getComponentPoolRegistry()->getBehaviourPools();

	for (auto& [scriptUID, pool] : behaviourPools) {
		std::string scriptName = componentTypeUIDToString(scriptUID);
		if (scriptName == "<unkown_component>") continue;
		Archive scriptArchive = scriptsCacheArchive.get(scriptName);

		uint32_t entityCount;
		scriptArchive.get("entityCount", entityCount);
		for (uint32_t i = 0; i < entityCount; ++i) {
			Archive entityArchive = scriptArchive.get(std::to_string(i));

			uint32_t entityId;
			uint32_t generation;
			if (!entityArchive.get("entityId", entityId) || !entityArchive.get("generation", generation)) continue;

			ECS::EntityHandle handle{ entityId, generation };
			Entity entity(&scene, handle);
			if (!scene.entityExists(entity)) continue;

			ComponentFactory::create(scriptName, entity, entityArchive);
		}
	}

	cacheSaved = false;
	scriptsCacheArchive.reset();
}