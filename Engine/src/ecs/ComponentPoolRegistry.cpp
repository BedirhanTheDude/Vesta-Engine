#include "ComponentPoolRegistry.h"

#include <ecs/EntityHandle.h>

#include <scene/components/TransformComponent.h>
#include <scene/components/CameraComponent.h>
#include <scene/components/MeshComponent.h>
#include <scene/components/MaterialComponent.h>
#include <scene/components/DirectionalLightComponent.h>
#include <scene/components/PointLightComponent.h>
#include <scene/components/RigidBodyComponent.h>

namespace ECS {
	ComponentPoolRegistry::ComponentPoolRegistry() {
		pools[componentTypeUID<TransformComponent>()] = &transforms;
		pools[componentTypeUID<CameraComponent>()] = &cameras;
		pools[componentTypeUID<MeshComponent>()] = &meshes;
		pools[componentTypeUID<MaterialComponent>()] = &materials;
		pools[componentTypeUID<DirectionalLightComponent>()] = &directionalLights;
		pools[componentTypeUID<PointLightComponent>()] = &pointLights;
		pools[componentTypeUID<RigidBodyComponent>()] = &rigidBodies;
	}

	ComponentPoolRegistry::~ComponentPoolRegistry() = default;

	void ComponentPoolRegistry::reset() {
		for (auto& [UID, pool] : pools)
			pool->reset();

		behaviourPools.clear();
	}

	void ComponentPoolRegistry::removeEntityComponents(const EntityHandle& handle) {
		for (auto& [UID, pool] : pools)
			pool->remove(handle);

		for (auto& [UID, pool] : behaviourPools)
			pool.remove(handle);
	}

	IComponentPool* ComponentPoolRegistry::getPool(uint32_t UID) {
		auto it = pools.find(UID);
		return it != pools.end() ? it->second : nullptr;
	}

	ComponentPoolRegistry::BehaviourPool* ComponentPoolRegistry::getBehaviourPool(uint32_t UID) {
		auto it = behaviourPools.find(UID);
		return it != behaviourPools.end() ? &it->second : nullptr;
	}

	ComponentPoolRegistry::BehaviourPool& ComponentPoolRegistry::getOrCreateBehaviourPool(uint32_t UID) {
		return behaviourPools[UID];
	}

	std::unordered_map<uint32_t, ComponentPoolRegistry::BehaviourPool>& ComponentPoolRegistry::getBehaviourPools() {
		return behaviourPools;
	}

	ComponentPool<Transform>& ComponentPoolRegistry::getTransforms() { return transforms; }
	ComponentPool<Camera>& ComponentPoolRegistry::getCameras() { return cameras; }
	ComponentPool<MeshData>& ComponentPoolRegistry::getMeshes() { return meshes; }
	ComponentPool<MaterialData>& ComponentPoolRegistry::getMaterials() { return materials; }
	ComponentPool<DirectionalLight>& ComponentPoolRegistry::getDirectionalLights() { return directionalLights; }
	ComponentPool<PointLight>& ComponentPoolRegistry::getPointLights() { return pointLights; }
	ComponentPool<RigidBody>& ComponentPoolRegistry::getRigidBodies() { return rigidBodies; }
}
