#pragma once

#include "EntityHandle.h"

namespace ECS {
	class IComponentPool {
	public:
		virtual ~IComponentPool() = default;

		virtual bool has(EntityHandle entity) const = 0;
		virtual void* addDefault(EntityHandle entity) = 0; // default-constructs the component, returns a pointer to it
		virtual void* getRaw(EntityHandle entity) = 0;
		virtual void remove(EntityHandle entity) = 0;
		virtual void reset() = 0;
	};
}
