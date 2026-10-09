#include "ScriptSystem.h"

#include <scene/Scene.h>
#include <scene/components/BehaviourComponent.h>
#include <ecs/ComponentPoolRegistry.h>

namespace ScriptSystem {

	void clearScripts(Scene& scene) {
		auto& behaviourPools = scene.getComponentPoolRegistry()->getBehaviourPools();
		for (auto& [UID, pool] : behaviourPools)
			pool.reset();
	}
}