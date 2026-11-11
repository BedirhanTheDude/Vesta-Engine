#include "ScriptSystem.h"

#include <scene/Scene.h>
#include <scene/components/BehaviourComponent.h>
#include <ecs/ComponentPoolRegistry.h>

namespace ScriptSystem {

	void clearScripts(Scene& scene) {
		auto& scriptPools = scene.getComponentPoolRegistry()->getScriptPools();
		for (auto& [UID, pool] : scriptPools)
			pool.reset();
	}
}