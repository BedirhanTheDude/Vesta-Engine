#include "ComponentReflection.h"

#include <scene/Entity.h>

#include <scene/components/CameraComponent.h>
#include <scene/components/DirectionalLightComponent.h>
#include <scene/components/PointLightComponent.h>
#include <scene/components/MeshComponent.h>
#include <scene/components/MaterialComponent.h>
#include <scene/components/RigidBodyComponent.h>

#include <scene/components/Camera.h>
#include <scene/components/DirectionalLight.h>
#include <scene/components/PointLight.h>
#include <scene/components/MeshData.h>
#include <scene/components/MaterialData.h>
#include <scene/components/RigidBody.h>
#include <scene/components/ProxyUtil.h>

#include <renderer/Material.h>
#include <physics/PhysicsMaterial.h>

#include <string>
#include <vector>

namespace {

	// plain data with no invariants to protect

	bool reflectCamera(const Entity& entity, ReflectedComponent& out) {
		Camera* camera = resolveComponent<Camera>(entity);
		if (!camera) return false;

		out.registerProperty("FOV", PropertyType::Float, &camera->fov);
		out.registerProperty("Near", PropertyType::Float, &camera->nearPlane);
		out.registerProperty("Far", PropertyType::Float, &camera->farPlane);

		return true;
	}

	bool reflectDirectionalLight(const Entity& entity, ReflectedComponent& out) {
		DirectionalLight* light = resolveComponent<DirectionalLight>(entity);
		if (!light) return false;

		out.registerProperty("color", PropertyType::Color, &light->color);

		out.registerProperty("ambientStrength", PropertyType::Float, &light->ambientStrength);
		out.registerProperty("diffuseStrength", PropertyType::Float, &light->diffuseStrength);
		out.registerProperty("specularStrength", PropertyType::Float, &light->specularStrength);

		out.registerProperty("shadowDistance", PropertyType::Float, &light->shadowDistance);
		out.registerProperty("shadowOrthoSize", PropertyType::Float, &light->shadowOrthoSize);
		out.registerProperty("shadowNear", PropertyType::Float, &light->shadowNear);
		out.registerProperty("shadowFar", PropertyType::Float, &light->shadowFar);

		return true;
	}

	bool reflectPointLight(const Entity& entity, ReflectedComponent& out) {
		PointLight* light = resolveComponent<PointLight>(entity);
		if (!light) return false;

		out.registerProperty("Color", PropertyType::Color, &light->color);

		out.registerProperty("Ambient Strength", PropertyType::Float, &light->ambientStrength);
		out.registerProperty("Diffuse Strength", PropertyType::Float, &light->diffuseStrength);
		out.registerProperty("Specular Strength", PropertyType::Float, &light->specularStrength);

		out.registerProperty("Constant", PropertyType::Float, &light->constant);
		out.registerProperty("Linear", PropertyType::Float, &light->linear);
		out.registerProperty("Quadratic", PropertyType::Float, &light->quadratic);

		return true;
	}

	bool reflectMesh(const Entity& entity, ReflectedComponent& out) {
		MeshData* data = resolveComponent<MeshData>(entity);
		if (!data) return false;

		static const std::vector<const char*> primitiveNames = { "none", "cube", "sphere", "quad" };

		// picking a primitive swaps the mesh, so it goes through the component instead of writing the field
		auto primitiveCallback = [entity](const void* ptr) {
			MeshComponent(entity).setPrimitive(static_cast<unsigned int>(*static_cast<const int*>(ptr)));
		};

		out.registerCallback("Primitive", CallbackPropertyType::Enum, &data->primitive, primitiveNames, primitiveCallback);
		out.registerPayload("Mesh", PayloadType::Mesh, &data->mesh);

		return true;
	}

	bool reflectMaterial(const Entity& entity, ReflectedComponent& out) {
		MaterialData* data = resolveComponent<MaterialData>(entity);
		if (!data) return false;

		for (unsigned int i = 0; i < data->materials.size(); i++) {
			Material* mat = data->materials[i].get();
			if (!mat) continue;

			std::string groupName = "Material " + std::to_string(i + 1);

			auto alphaCallback = [mat]() {
				mat->transparent = mat->alpha < 1.0f;
			};

			out.registerProperty("Albedo", PropertyType::Color, &mat->albedo, nullptr, i, groupName);
			out.registerProperty("Emission", PropertyType::Color, &mat->emission, nullptr, i, groupName);
			out.registerProperty("Shininess", PropertyType::Float, &mat->shininess, nullptr, i, groupName);
			out.registerProperty("Ambient Ref", PropertyType::Float, &mat->ambientReflectance, nullptr, i, groupName);
			out.registerProperty("Specular Ref", PropertyType::Float, &mat->specularReflectance, nullptr, i, groupName);
			out.registerProperty("Alpha", PropertyType::Float, &mat->alpha, alphaCallback, i, groupName);

			out.registerPayload("Texture", PayloadType::Texture, &mat->diffuseTexture, i, groupName);
		}

		return true;
	}

	bool reflectRigidBody(const Entity& entity, ReflectedComponent& out) {
		RigidBody* rb = resolveComponent<RigidBody>(entity);
		if (!rb) return false;

		auto typeCallback = [entity](const void* ptr) {
			RigidBodyComponent(entity).setType(static_cast<RigidBodyType>(*static_cast<const int*>(ptr)));
		};

		auto convexCallback = [entity](const void* ptr) {
			RigidBodyComponent(entity).setForceConvex(*static_cast<const bool*>(ptr));
		};

		auto triangleCallback = [entity](const void* ptr) {
			RigidBodyComponent(entity).setUseTriangleMesh(*static_cast<const bool*>(ptr));
		};

		auto linearDampingCallback = [entity](const void* ptr) {
			RigidBodyComponent(entity).setLinearDamping(*static_cast<const float*>(ptr));
		};

		auto angularDampingCallback = [entity](const void* ptr) {
			RigidBodyComponent(entity).setAngularDamping(*static_cast<const float*>(ptr));
		};

		auto dynamicFrictionCallback = [entity](const void* ptr) {
			RigidBodyComponent(entity).setDynamicFriction(*static_cast<const float*>(ptr));
		};

		auto staticFrictionCallback = [entity](const void* ptr) {
			RigidBodyComponent(entity).setStaticFriction(*static_cast<const float*>(ptr));
		};

		auto restitutionCallback = [entity](const void* ptr) {
			RigidBodyComponent(entity).setRestitution(*static_cast<const float*>(ptr));
		};

		out.registerCallback("Body Type", CallbackPropertyType::Enum, &rb->type, RigidBodyType_t::values(), typeCallback);
		out.registerCallback("Force Convex", CallbackPropertyType::Bool, &rb->forceConvex, convexCallback);
		out.registerCallback("Use Triangle Mesh", CallbackPropertyType::Bool, &rb->useTriangleMesh, triangleCallback);
		out.registerCallback("Linear Damping", CallbackPropertyType::Float, &rb->linearDamping, linearDampingCallback);
		out.registerCallback("Angular Damping", CallbackPropertyType::Float, &rb->angularDamping, angularDampingCallback);

		// the material only exists once the body was attached
		if (rb->material) {
			out.registerCallback("Dynamic Friction", CallbackPropertyType::Float, &rb->material->dynamicFriction, dynamicFrictionCallback);
			out.registerCallback("Static Friction", CallbackPropertyType::Float, &rb->material->staticFriction, staticFrictionCallback);
			out.registerCallback("Restitution", CallbackPropertyType::Float, &rb->material->restitution, restitutionCallback);
		}

		return true;
	}
}

namespace {

	template<typename D>
	bool fingerprintOf(const Entity& entity, std::vector<const void*>& fingerprint) {
		D* data = resolveComponent<D>(entity);
		if (!data) return false;

		fingerprint.push_back(data);
		return true;
	}

	// while these are unchanged the reflection is still valid, when one differs it has to be rebuilt
	bool gatherFingerprint(const Entity& entity, unsigned int UID, std::vector<const void*>& fingerprint) {
		if (!entity.isValid()) return false;

		if (UID == componentTypeUID<CameraComponent>()) return fingerprintOf<Camera>(entity, fingerprint);
		if (UID == componentTypeUID<DirectionalLightComponent>()) return fingerprintOf<DirectionalLight>(entity, fingerprint);
		if (UID == componentTypeUID<PointLightComponent>()) return fingerprintOf<PointLight>(entity, fingerprint);
		if (UID == componentTypeUID<MeshComponent>()) return fingerprintOf<MeshData>(entity, fingerprint);

		if (UID == componentTypeUID<MaterialComponent>()) {
			MaterialData* data = resolveComponent<MaterialData>(entity);
			if (!data) return false;

			fingerprint.push_back(data);
			for (const auto& material : data->materials)
				fingerprint.push_back(material.get()); // the material properties point into these

			return true;
		}

		if (UID == componentTypeUID<RigidBodyComponent>()) {
			RigidBody* rb = resolveComponent<RigidBody>(entity);
			if (!rb) return false;

			fingerprint.push_back(rb);
			fingerprint.push_back(rb->material.get()); // the friction/restitution properties point into this

			return true;
		}

		return false;
	}
}

namespace ComponentReflection {

	bool isCurrent(const Entity& entity, unsigned int componentUID, const ReflectedComponent& built) {
		thread_local std::vector<const void*> current;
		current.clear();

		if (!gatherFingerprint(entity, componentUID, current)) return false;

		return current == built.fingerprint;
	}

	bool build(const Entity& entity, unsigned int componentUID, ReflectedComponent& out) {
		out.clear();

		if (!gatherFingerprint(entity, componentUID, out.fingerprint)) return false;

		if (componentUID == componentTypeUID<CameraComponent>()) return reflectCamera(entity, out);
		if (componentUID == componentTypeUID<DirectionalLightComponent>()) return reflectDirectionalLight(entity, out);
		if (componentUID == componentTypeUID<PointLightComponent>()) return reflectPointLight(entity, out);
		if (componentUID == componentTypeUID<MeshComponent>()) return reflectMesh(entity, out);
		if (componentUID == componentTypeUID<MaterialComponent>()) return reflectMaterial(entity, out);
		if (componentUID == componentTypeUID<RigidBodyComponent>()) return reflectRigidBody(entity, out);

		return false;
	}
}
