#include "ComponentPoolRegistry.h"

#include <ecs/EntityHandle.h>

#include <scene/components/TransformComponent.h>
#include <scene/components/CameraComponent.h>
#include <scene/components/MeshComponent.h>
#include <scene/components/MaterialComponent.h>
#include <scene/components/DirectionalLightComponent.h>
#include <scene/components/PointLightComponent.h>
#include <scene/components/RigidBodyComponent.h>

#include <scene/components/ComponentFactory.h>

namespace ECS {
	ComponentPoolRegistry::ComponentPoolRegistry() {
		m_Pools[componentTypeUID<TransformComponent>()] = &m_Transforms;
		m_Pools[componentTypeUID<CameraComponent>()] = &m_Cameras;
		m_Pools[componentTypeUID<MeshComponent>()] = &m_Meshes;
		m_Pools[componentTypeUID<MaterialComponent>()] = &m_Materials;
		m_Pools[componentTypeUID<DirectionalLightComponent>()] = &m_DirectionalLights;
		m_Pools[componentTypeUID<PointLightComponent>()] = &m_PointLights;
		m_Pools[componentTypeUID<RigidBodyComponent>()] = &m_RigidBodies;
	}

	ComponentPoolRegistry::~ComponentPoolRegistry() = default;

	void ComponentPoolRegistry::reset() {
		for (auto& [UID, pool] : m_Pools)
			pool->reset();

		m_ScriptPools.clear();
	}

	void ComponentPoolRegistry::removeEntityComponents(const EntityHandle& handle) {
		for (auto& [UID, pool] : m_Pools)
			pool->remove(handle);

		for (auto& [UID, pool] : m_ScriptPools)
			pool.remove(handle);
	}

	IComponentPool* ComponentPoolRegistry::getPool(uint32_t UID) {
		auto it = m_Pools.find(UID);
		return it != m_Pools.end() ? it->second : nullptr;
	}

	ScriptPool* ComponentPoolRegistry::getScriptPool(uint32_t UID) {
		auto it = m_ScriptPools.find(UID);
		return it != m_ScriptPools.end() ? &it->second : nullptr;
	}

	ScriptPool* ComponentPoolRegistry::getOrCreateScriptPool(uint32_t UID) {
		auto it = m_ScriptPools.find(UID);

		if (it != m_ScriptPools.end())
			return &it->second;

		const ScriptTypeInfo* info = ComponentFactory::getScriptTypeInfo(UID);
		if (!info) return nullptr;

		auto [insertedIt, success] = m_ScriptPools.try_emplace(UID, *info);

		return &insertedIt->second;
	}

	std::unordered_map<uint32_t, ScriptPool>& ComponentPoolRegistry::getScriptPools() {
		return m_ScriptPools;
	}

	ComponentPool<Transform>& ComponentPoolRegistry::getTransforms() { return m_Transforms; }
	ComponentPool<Camera>& ComponentPoolRegistry::getCameras() { return m_Cameras; }
	ComponentPool<MeshData>& ComponentPoolRegistry::getMeshes() { return m_Meshes; }
	ComponentPool<MaterialData>& ComponentPoolRegistry::getMaterials() { return m_Materials; }
	ComponentPool<DirectionalLight>& ComponentPoolRegistry::getDirectionalLights() { return m_DirectionalLights; }
	ComponentPool<PointLight>& ComponentPoolRegistry::getPointLights() { return m_PointLights; }
	ComponentPool<RigidBody>& ComponentPoolRegistry::getRigidBodies() { return m_RigidBodies; }
}
