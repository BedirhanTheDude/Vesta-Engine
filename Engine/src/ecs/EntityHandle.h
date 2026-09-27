#pragma once

#include <cstdint>

namespace ECS {

	static constexpr uint32_t INVALID_ENTITY_INDEX = UINT32_MAX;
	static constexpr uint32_t INVALID_ENTITY_GENERATION = UINT32_MAX;

	struct EntityHandle {
		uint32_t entityId;
		uint32_t generation = 0;

		bool operator==(const EntityHandle& other) const {
			return entityId == other.entityId && generation == other.generation;
		}

		bool operator<(const EntityHandle& other) const {
			if (entityId != other.entityId) return entityId < other.entityId;
			return generation < other.generation;
		}

		bool operator<=(const EntityHandle& other) const {
			return *this < other || *this == other;
		}

		bool operator>(const EntityHandle& other) const {
			return other < *this;
		}

		bool operator>=(const EntityHandle& other) const {
			return other <= *this;
		}

		operator uint32_t() const {
			return entityId;
		}

		bool isInvalid() const {
			return entityId == INVALID_ENTITY_INDEX || generation == INVALID_ENTITY_GENERATION;
		}
	};

	static constexpr EntityHandle INVALID_ENTITY_HANDLE = { INVALID_ENTITY_INDEX, INVALID_ENTITY_GENERATION };
}