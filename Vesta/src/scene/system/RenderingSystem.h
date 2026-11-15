#pragma once

#include <scene/components/MaterialData.h>
#include <scene/components/MeshData.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

class Scene;

struct DirectionalLight;
struct Transform;

namespace RenderingSystem {

	struct PointLightData {
		glm::vec3 position = glm::vec3(0.0f); // world
		glm::vec3 color = glm::vec3(1.0f);
		float ambientIntensity = 0.1f;
		float diffuseIntensity = 1.0f;
		float specularIntensity = 0.8f;
		float constant = 1.0f;
		float linear = 0.09f;
		float quadratic = 0.032f;
	};

	struct DirectionalLightData {
		glm::vec3 direction = glm::vec3(0.707f, 0.707f, 0.0f);
		glm::vec3 color = glm::vec3(1.0f);
		float ambientStrength = 0.2f;
		float diffuseStrength = 1.0f;
		float specularStrength = 0.5f;

		bool hasDirLight = false;
	};	

	struct ShadowFrustum {
		float size = 40.0f;      // width and height of the covered area, world units
		float nearPlane = 0.1f; // depth range measured from the light
		float farPlane = 200.0f;
	};

	struct ShadowData {
		ShadowFrustum frustum;
		glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
	};

	struct Renderable {
		uint32_t entityId;
		glm::mat4 model;
		MeshData mesh;
		MaterialData material;
	};

	struct SceneRenderView {
		std::vector<Renderable> renderables;
		std::vector<PointLightData> pointLightData;

		uint32_t renderableCount = 0;

		DirectionalLightData dirLightData;
		ShadowData shadowData;

		bool isRenderable = false;

		void reset() {
			renderables.clear();
			pointLightData.clear();

			renderableCount = 0;

			dirLightData.hasDirLight = false; // essentially invalidates all data inside
			// shadow data unused if no dir light present

			isRenderable = false;
		}
	};

	const SceneRenderView* getSceneRenderView(const Scene& scene);
	
	glm::mat4 getActiveCameraView(const Scene& scene);
	glm::mat4 getActiveCameraProj(const Scene& scene);

	glm::vec3 getLightDirection(const Transform& transform);

	// The shadow box follows the camera: the light sits shadowDistance behind the box center, looks at it and
	// covers a size x size area over the depth range [near, far].
	// NOTE: I am not sure if I like the current shadow map boundary
	glm::mat4 getLightSpaceMatrix(const DirectionalLight& light, const Transform& transform, const ShadowFrustum& frustum,
		const glm::vec3& camPos, const glm::vec3& camForward = glm::vec3(0.0f));
	ShadowFrustum buildShadowFrustum(const DirectionalLight& light);
}