#include "RenderingSystem.h"

#include <ecs/EntityHandle.h>
#include <ecs/ComponentPool.h>
#include <ecs/ComponentPoolRegistry.h>

#include <scene/system/TransformSystem.h>
#include <scene/system/CameraSystem.h>

#include <scene/components/Transform.h>
#include <scene/components/MeshData.h>
#include <scene/components/MaterialData.h>
#include <scene/components/PointLight.h>
#include <scene/components/DirectionalLight.h>
#include <scene/components/Camera.h>

#include <scene/Scene.h>
#include <scene/Entity.h>

#include <glm/glm.hpp>

#include <utility>
#include <algorithm>

namespace RenderingSystem {

	const SceneRenderView* getSceneRenderView(const Scene& scene) {
		static SceneRenderView renderView;
		
		renderView.reset();

		Scene* mutableScene = const_cast<Scene*>(&scene);

		ECS::ComponentPoolRegistry* registry = scene.getComponentPoolRegistry();

		ECS::ComponentPool<Transform>& transforms = registry->getTransforms();
		ECS::ComponentPool<MeshData>& meshes = registry->getMeshes();
		ECS::ComponentPool<MaterialData>& materials = registry->getMaterials();
		ECS::ComponentPool<PointLight>& pointLights = registry->getPointLights();
		ECS::ComponentPool<DirectionalLight>& directionalLights = registry->getDirectionalLights();

		Transform* cameraTransform = transforms.get(scene.getActiveCameraEntity().getHandle());
		if (!cameraTransform) {
			renderView.isRenderable = false;
			return &renderView;
		}

		// my cpu cries here
		for (auto [entity, mesh] : meshes) {
			if (!mesh.mesh) continue;

			MaterialData* material = materials.get(entity);

			if (!material) continue; // needs both mat and mesh to render

			glm::mat4 model = TransformSystem::getMatrix(*transforms.get(entity), mutableScene);

			renderView.models.push_back(model);
			renderView.meshData.push_back(mesh);
			renderView.materialData.push_back(*material);

			++renderView.entityCount;
		}

		// build point light buffer
		for (auto [entity, pointLight] : pointLights) {
			PointLightData pointLightData;
			
			Transform& transform = *transforms.get(entity);
			pointLightData.position = TransformSystem::getWorldPosition(transform, mutableScene);
			pointLightData.color = pointLight.color;
			pointLightData.ambientIntensity = pointLight.ambientStrength;
			pointLightData.diffuseIntensity = pointLight.diffuseStrength;
			pointLightData.specularIntensity = pointLight.specularStrength;
			pointLightData.constant = pointLight.constant;
			pointLightData.linear = pointLightData.linear;
			pointLightData.quadratic = pointLightData.quadratic;

			renderView.pointLightData.push_back(std::move(pointLightData));
		}

		DirectionalLight* dirLight = nullptr;
		ECS::EntityHandle dirLightEntity;
		for (auto [entity, component] : directionalLights) {
			dirLightEntity = entity;
			dirLight = &component; // get the first dir light (if there is any) and get out
			break;
		}

		if (dirLight) {
			Transform& dirLightTransform = *transforms.get(dirLightEntity);

			renderView.dirLightData.direction = getLightDirection(dirLightTransform);
			renderView.dirLightData.color = dirLight->color;
			renderView.dirLightData.ambientStrength = dirLight->ambientStrength;
			renderView.dirLightData.diffuseStrength = dirLight->diffuseStrength;
			renderView.dirLightData.specularStrength = dirLight->specularStrength;
			renderView.dirLightData.hasDirLight = true;

			renderView.shadowData.frustum = buildShadowFrustum(*dirLight);
			renderView.shadowData.lightSpaceMatrix = getLightSpaceMatrix(*dirLight, dirLightTransform,
				renderView.shadowData.frustum, cameraTransform->position, TransformSystem::forward(*cameraTransform));
		}
		else
			renderView.dirLightData.hasDirLight = false;

		renderView.isRenderable = true;

		return &renderView;
	}

	glm::mat4 getActiveCameraView(const Scene& scene) {
		glm::mat4 view(1.0f);

		auto* registry = scene.getComponentPoolRegistry();

		ECS::EntityHandle cameraEntity = scene.getActiveCameraEntity().getHandle();
		Transform* cameraTransform = registry->getTransforms().get(cameraEntity);

		if (cameraTransform)
			view = CameraSystem::getViewMatrix(*cameraTransform);

		return view;
	}

	glm::mat4 getActiveCameraProj(const Scene& scene) {
		glm::mat4 proj(1.0f);

		auto* registry = scene.getComponentPoolRegistry();

		ECS::EntityHandle cameraEntity = scene.getActiveCameraEntity().getHandle();
		Camera* camera = registry->getCameras().get(cameraEntity);

		if (camera)
			proj = CameraSystem::getProjectionMatrix(*camera);

		return proj;
	}

	glm::vec3 getLightDirection(const Transform& transform) {
		return TransformSystem::forward(transform);
	}

	glm::mat4 getLightSpaceMatrix(const DirectionalLight& light, const Transform& transform,
		const ShadowFrustum& frustum, const glm::vec3& camPos, const glm::vec3& camForward) {
		float halfSize = frustum.size * 0.5f;

		glm::vec3 dir = glm::normalize(getLightDirection(transform));

		glm::vec3 lookPerpendicular = camForward - dir * glm::dot(camForward, dir);
		glm::vec3 center = camPos + lookPerpendicular * (halfSize * 0.5f);

		glm::vec3 lightPos = center - dir * light.shadowDistance;

		// looking straight up or down is parallel to the usual up vector, which makes lookAt produce NaNs
		glm::vec3 up = std::abs(glm::dot(dir, glm::vec3(0, 1, 0))) > 0.999f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);

		glm::mat4 lightView = glm::lookAt(lightPos, center, up);
		glm::mat4 lightProj = glm::ortho(-halfSize, halfSize, -halfSize, halfSize, frustum.nearPlane, frustum.farPlane);

		return lightProj * lightView;
	}

	ShadowFrustum buildShadowFrustum(const DirectionalLight& light) {
		ShadowFrustum frustum;
		frustum.size = std::max(light.shadowOrthoSize, 0.001f);
		frustum.nearPlane = std::max(light.shadowNear, 0.001f);
		frustum.farPlane = std::max(light.shadowFar, frustum.nearPlane + 0.001f);

		return frustum;
	}
}