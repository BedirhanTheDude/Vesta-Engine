#include "EntityManager.h"

#include <cstdint>

namespace ECS {
	EntityHandle EntityManager::create() {
		if (!m_FreeList.empty()) {
			uint32_t index = m_FreeList.back();
			m_FreeList.pop_back();

			m_Alive[index] = 1;
			return { index, m_Generations[index] };
		}

		// entityId grows monotonically, starts at generation 0
		m_Alive.emplace_back(1);
		m_Generations.emplace_back(0);

		return { static_cast<uint32_t>(m_Alive.size() - 1), 0 };
	}

	void EntityManager::destroy(EntityHandle entity) {
		if (!isAlive(entity)) return; // also rejects a stale/already-dead handle

		uint32_t index = entity;
		m_Alive[index] = 0;
		++m_Generations[index]; // invalidates every handle still referring to this generation
		m_FreeList.push_back(index);
	}

	bool EntityManager::isAlive(EntityHandle entity) const {
		if (entity >= m_Alive.size()) return false;

		return static_cast<bool>(m_Alive[entity]) && m_Generations[entity] == entity.generation;
	}

	uint32_t EntityManager::getGeneration(uint32_t id) const {
		if (id >= m_Alive.size() || !m_Alive[id]) return INVALID_ENTITY_GENERATION;
		// alive and generation has same size, it's an invariant

		return m_Generations[id];
	}

	void EntityManager::reset() {
		m_FreeList.clear();

		for (uint32_t index = static_cast<uint32_t>(m_Alive.size()); index-- > 0;) {
			m_Alive[index] = 0;
			++m_Generations[index];
			m_FreeList.push_back(index);
		}
	}
}