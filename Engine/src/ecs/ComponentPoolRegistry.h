#pragma once

#include <ecs/IComponentPool.h>
#include <ecs/ComponentPool.h>
#include <ecs/ScriptPool.h>

#include <scene/components/Transform.h>
#include <scene/components/Camera.h>
#include <scene/components/DirectionalLight.h>
#include <scene/components/PointLight.h>
#include <scene/components/MeshData.h>
#include <scene/components/MaterialData.h>
#include <scene/components/RigidBody.h>
#include <scene/components/BehaviourComponent.h>

#include <memory>
#include <unordered_map>
#include <type_traits>
#include <cstdint>

namespace ECS {
	struct EntityHandle;

	class ComponentPoolRegistry {
	public:
		ComponentPoolRegistry();
		~ComponentPoolRegistry();

		ComponentPoolRegistry(const ComponentPoolRegistry&) = delete;
		ComponentPoolRegistry& operator=(const ComponentPoolRegistry&) = delete;

		void reset();
		void removeEntityComponents(const EntityHandle& handle);

		// Type-erased access to a built-in pool by its proxy's component type UID, nullptr if unknown.
		IComponentPool* getPool(uint32_t UID);

		ScriptPool* getScriptPool(uint32_t UID);
		ScriptPool* getOrCreateScriptPool(uint32_t UID);
		std::unordered_map<uint32_t, ScriptPool>& getScriptPools();

		ComponentPool<Transform>& getTransforms();
		ComponentPool<Camera>& getCameras();
		ComponentPool<MeshData>& getMeshes();
		ComponentPool<MaterialData>& getMaterials();
		ComponentPool<DirectionalLight>& getDirectionalLights();
		ComponentPool<PointLight>& getPointLights();
		ComponentPool<RigidBody>& getRigidBodies();

		// Lookup by data type, used by the proxies: resolveComponent<Camera>(entity)
		template<typename T>
		ComponentPool<T>& get() {
			if constexpr (std::is_same_v<T, Transform>) return m_Transforms;
			else if constexpr (std::is_same_v<T, Camera>) return m_Cameras;
			else if constexpr (std::is_same_v<T, MeshData>) return m_Meshes;
			else if constexpr (std::is_same_v<T, MaterialData>) return m_Materials;
			else if constexpr (std::is_same_v<T, DirectionalLight>) return m_DirectionalLights;
			else if constexpr (std::is_same_v<T, PointLight>) return m_PointLights;
			else if constexpr (std::is_same_v<T, RigidBody>) return m_RigidBodies;
			else static_assert(!sizeof(T), "No component pool registered for this type");
		}

	private:
		ComponentPool<Transform> m_Transforms;
		ComponentPool<Camera> m_Cameras;
		ComponentPool<MeshData> m_Meshes;
		ComponentPool<MaterialData> m_Materials;
		ComponentPool<DirectionalLight> m_DirectionalLights;
		ComponentPool<PointLight> m_PointLights;
		ComponentPool<RigidBody> m_RigidBodies;

		std::unordered_map<uint32_t, IComponentPool*> m_Pools; // proxy UID -> pool above, pointers into this object
		std::unordered_map<uint32_t, ScriptPool> m_ScriptPools; // script UID -> pool, created on first use
	};
}
