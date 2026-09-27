#pragma once

#include <ecs/ComponentPool.h>
#include <ecs/IComponentPool.h>

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
		// Behaviours are heap objects
		using BehaviourPool = ComponentPool<std::unique_ptr<BehaviourComponent>>;

		ComponentPoolRegistry();
		~ComponentPoolRegistry();

		ComponentPoolRegistry(const ComponentPoolRegistry&) = delete;
		ComponentPoolRegistry& operator=(const ComponentPoolRegistry&) = delete;

		void reset();
		void removeEntityComponents(const EntityHandle& handle);

		// Type-erased access to a built-in pool by its proxy's component type UID, nullptr if unknown.
		IComponentPool* getPool(uint32_t UID);

		BehaviourPool* getBehaviourPool(uint32_t UID);
		BehaviourPool& getOrCreateBehaviourPool(uint32_t UID);
		std::unordered_map<uint32_t, BehaviourPool>& getBehaviourPools();

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
			if constexpr (std::is_same_v<T, Transform>) return transforms;
			else if constexpr (std::is_same_v<T, Camera>) return cameras;
			else if constexpr (std::is_same_v<T, MeshData>) return meshes;
			else if constexpr (std::is_same_v<T, MaterialData>) return materials;
			else if constexpr (std::is_same_v<T, DirectionalLight>) return directionalLights;
			else if constexpr (std::is_same_v<T, PointLight>) return pointLights;
			else if constexpr (std::is_same_v<T, RigidBody>) return rigidBodies;
			else static_assert(!sizeof(T), "No component pool registered for this type");
		}

	private:
		ComponentPool<Transform> transforms;
		ComponentPool<Camera> cameras;
		ComponentPool<MeshData> meshes;
		ComponentPool<MaterialData> materials;
		ComponentPool<DirectionalLight> directionalLights;
		ComponentPool<PointLight> pointLights;
		ComponentPool<RigidBody> rigidBodies;

		std::unordered_map<uint32_t, IComponentPool*> pools; // proxy UID -> pool above, pointers into this object
		std::unordered_map<uint32_t, BehaviourPool> behaviourPools; // script UID -> pool, created on first use
	};
}
