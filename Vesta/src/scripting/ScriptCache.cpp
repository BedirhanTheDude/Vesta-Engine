#include "ScriptCache.h"

#include <persistence/Archive.h>
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
	auto& scriptPools = scene.getComponentPoolRegistry()->getScriptPools();
	scriptsCacheArchive = Archive();

	for (auto& [scriptUID, pool] : scriptPools) {
		Archive scriptArchive;
		uint32_t index = 0;
		for (auto& [entityHandle, script] : pool) {
			Archive entityArchive;
			entityArchive.set("entityId", entityHandle.entityId);
			entityArchive.set("generation", entityHandle.generation);

			script.serialize(entityArchive);
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

	for (const std::string& scriptName : ComponentFactory::getScriptNames()) {
		if (scriptName == "<unkown_component>") continue;
		Archive scriptArchive = scriptsCacheArchive.get(scriptName);

		uint32_t entityCount = 0;
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