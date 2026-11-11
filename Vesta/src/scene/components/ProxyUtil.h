#pragma once

#include <scene/Entity.h>
#include <scene/Scene.h>

#include <ecs/EntityHandle.h>
#include <ecs/ComponentPool.h>
#include <ecs/ComponentPoolRegistry.h>

// pointer to data type D belonging to this entity
template<typename D>
D* resolveComponent(const Entity& entity) {
	if (!entity.isValid()) return nullptr;

	return entity.getScene().getComponentPoolRegistry()->get<D>().get(entity.getHandle());
}

inline Scene* sceneOf(const Entity& entity) {
	return entity.isValid() ? &entity.getScene() : nullptr;
}
