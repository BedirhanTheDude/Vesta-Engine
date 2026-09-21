#include "EntityManager.h"

#include <cstdint>

namespace ECS {
	EntityHandle EntityManager::create() {
		if (!freeList.empty()) {
			uint32_t index = freeList.back();
			freeList.pop_back();

			alive[index] = 1;
			return { index, generations[index] };
		}

		// entityId grows monotonically, starts at generation 0
		alive.emplace_back(1);
		generations.emplace_back(0);

		return { static_cast<uint32_t>(alive.size() - 1), 0 };
	}

	void EntityManager::destroy(EntityHandle entity) {
		if (!isAlive(entity)) return; // also rejects a stale/already-dead handle

		uint32_t index = entity;
		alive[index] = 0;
		++generations[index]; // invalidates every handle still referring to this generation
		freeList.push_back(index);
	}

	bool EntityManager::isAlive(EntityHandle entity) const {
		if (entity >= alive.size()) return false;

		return static_cast<bool>(alive[entity]) && generations[entity] == entity.generation;
	}

	uint32_t EntityManager::getGeneration(uint32_t id) const {
		if (id >= alive.size() || !alive[id]) return INVALID_ENTITY_GENERATION;
		// alive and generation has same size, it's an invariant

		return generations[id];
	}

	void EntityManager::reset() {
		freeList.clear();

		for (uint32_t index = static_cast<uint32_t>(alive.size()); index-- > 0;) {
			alive[index] = 0;
			++generations[index];
			freeList.push_back(index);
		}
	}
}