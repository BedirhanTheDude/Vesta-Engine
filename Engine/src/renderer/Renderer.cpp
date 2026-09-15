#include "Renderer.h"

#include <memory>

#include <scene/Scene.h>
#include <scene/Entity.h>
#include <scene/components/TransformComponent.h>
#include <scene/components/MeshComponent.h>
#include <scene/components/MaterialComponent.h>
#include <scene/components/CameraComponent.h>
#include <scene/components/DirectionalLightComponent.h>
#include <scene/components/PointLightComponent.h>

#include <renderer/ShaderProgram.h>
#include <renderer/Mesh.h>
#include <renderer/Material.h>

#include <glad/glad.h>


// DO NOT change order of this struct pls, memory alignment may break
struct PointLightGPU {
	glm::vec3 position;
	float ambientIntensity;
	glm::vec3 color;
	float diffuseIntensity;
	float specularIntensity;
	float constant;
	float linear;
	float quadratic;
};

static std::vector<PointLightGPU> buildPointLightBuffer(
	const std::vector<PointLightComponent*>& pointLightComponents) {
	std::vector<PointLightGPU> gpuBuffer;

	for (auto* plComp : pointLightComponents) {
		PointLightGPU pointLight;
		pointLight.position = plComp->getEntity()->transform.getPosition();
		pointLight.ambientIntensity = plComp->ambientStrength;
		pointLight.color = plComp->color;
		pointLight.diffuseIntensity = plComp->diffuseStrength;
		pointLight.specularIntensity = plComp->specularStrength;
		pointLight.constant = plComp->constant;
		pointLight.linear = plComp->linear;
		pointLight.quadratic = plComp->quadratic;

		gpuBuffer.push_back(pointLight);
	}

	return gpuBuffer;
}

static bool hasAnyTransparent(const MaterialComponent* matc) {
	int i = 0;
	int count = matc->getMaterialCount();
	while (i < count) {
		Material* m = matc->getMaterial(i++);
		if (m->transparent) return true;
	}
	return false;
}

Renderer::Renderer() {
	initShadowMap();

	depthShader = std::make_unique<ShaderProgram>("depth");
}

Renderer::~Renderer() {
	if (shadowFBO) glDeleteFramebuffers(1, &shadowFBO);
	if (shadowDepthTexture) glDeleteTextures(1, &shadowDepthTexture);
}

void Renderer::initShadowMap() {
	glGenFramebuffers(1, &shadowFBO);

	glGenTextures(1, &shadowDepthTexture);
	glBindTexture(GL_TEXTURE_2D, shadowDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
							GL_TEXTURE_2D, shadowDepthTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::render(const Scene& scene) {
    CameraComponent* cam = scene.getActiveCamera();
    if (!cam) return;
    render(scene, cam->getViewMatrix(), cam->getProjectionMatrix());
}

void Renderer::render(const Scene& scene, const glm::mat4& view, const glm::mat4& proj) {
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	GLint previousFBO;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFBO);

	glm::vec3 camPos = glm::vec3(glm::inverse(view)[3]);

	const DirectionalLightComponent* dirLight = nullptr;
	std::vector<PointLightComponent*> pointLights;

	for (const auto& [entityID, entityPtr] : scene.getEntities()) {
		if (!dirLight)
			dirLight = entityPtr->getComponent<DirectionalLightComponent>();
		auto* pl = entityPtr->getComponent<PointLightComponent>();
		if (pl) pointLights.push_back(pl);
	}

	glm::mat4 lightSpaceMatrix(1.0f);

	if (dirLight) {
		lightSpaceMatrix = dirLight->getLightSpaceMatrix(camPos);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		depthShader->use();

		for (const auto& [entityID, entityPtr] : scene.getEntities()) {
			const MeshComponent* mc = entityPtr->getComponent<MeshComponent>();
			const MaterialComponent* matc = entityPtr->getComponent<MaterialComponent>();
			if (!mc || !matc || !mc->getMesh()) continue;

			drawEntityDepth(*entityPtr, mc, lightSpaceMatrix);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);
	}

	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	glEnable(GL_DEPTH_TEST);
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (const auto& [entityID, entityPtr] : scene.getEntities()) {
		const MeshComponent* mc = entityPtr->getComponent<MeshComponent>();
		const MaterialComponent* matc = entityPtr->getComponent<MaterialComponent>();
		if (!mc || !matc) continue;

		drawEntity(*entityPtr, mc, matc, view, proj, camPos,
			dirLight, pointLights, lightSpaceMatrix, false);
	}

	std::vector<const Entity*> transparentEntities;
	for (const auto& [entityID, entityPtr] : scene.getEntities()) {
		const MaterialComponent* matc = entityPtr->getComponent<MaterialComponent>();
		if (matc && hasAnyTransparent(matc))
			transparentEntities.push_back(entityPtr.get());
	}

	std::sort(transparentEntities.begin(), transparentEntities.end(),
		[&camPos](const Entity* a, const Entity* b) {
			return glm::length(a->transform.getWorldPosition() - camPos) >
				glm::length(b->transform.getWorldPosition() - camPos);
		});

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	for (const Entity* entity : transparentEntities) {
		const MeshComponent* mc = entity->getComponent<MeshComponent>();
		const MaterialComponent* matc = entity->getComponent<MaterialComponent>();
		if (!mc || !matc) continue;
		drawEntity(*entity, mc, matc, view, proj, camPos,
			dirLight, pointLights, lightSpaceMatrix, true);
	}

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void Renderer::drawEntity(const Entity& entity,
	const MeshComponent* meshComponent,
	const MaterialComponent* materialComponent,
	const glm::mat4& view,
	const glm::mat4& proj,
	const glm::vec3& cameraPosition,
	const DirectionalLightComponent* dirLight,
	const std::vector<PointLightComponent*> pointLights,
	const glm::mat4& lightSpaceMatrix,
	bool transparentOnly) const {

	if (!meshComponent->getMesh()) return;

	const glm::mat4 model = entity.getTransform().getMatrix();

	auto drawWithMaterial = [&](Material& material) {
		ShaderProgram& shader = *material.shader;
		shader.use();
		material.apply();

		shader.setMat4("uModel", model);
		shader.setMat4("uView", view);
		shader.setMat4("uProj", proj);
		shader.setMat4("uLightSpaceMatrix", lightSpaceMatrix);
		shader.setVec3("uViewPos", cameraPosition);

		if (dirLight) {
			shader.setBool("uHasDirectionalLight", true);
			shader.setVec3("uDirectionalLight.direction", dirLight->getDirection());
			shader.setVec3("uDirectionalLight.color", dirLight->color);
			shader.setFloat("uDirectionalLight.ambientIntensity", dirLight->ambientStrength);
			shader.setFloat("uDirectionalLight.diffuseIntensity", dirLight->diffuseStrength);
			shader.setFloat("uDirectionalLight.specularIntensity", dirLight->specularStrength);
		}
		else {
			shader.setBool("uHasDirectionalLight", false);
		}

		auto buffer = buildPointLightBuffer(pointLights);
		shader.setSSBO<PointLightGPU>(0, &lightSSBO, "uPointLightCount", buffer);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, shadowDepthTexture);
		shader.setInt("uShadowMap", 4);
	};

	const auto& submeshes = meshComponent->getMesh()->getSubMeshes();

	if (submeshes.empty()) {
		Material* mat = materialComponent->getMaterial(0);
		if (!mat || !mat->shader) return;
		if (mat->transparent != transparentOnly) return; // skip wrong pass
		drawWithMaterial(*mat);
		meshComponent->getMesh()->draw();
	}
	else {
		for (int i = 0; i < static_cast<int>(submeshes.size()); i++) {
			Material* mat = materialComponent->getMaterial(submeshes[i].materialIndex);
			if (!mat) mat = materialComponent->getMaterial(0);
			if (!mat || !mat->shader) continue;
			if (mat->transparent != transparentOnly) continue; // skip wrong pass
			drawWithMaterial(*mat);
			meshComponent->getMesh()->drawSubMesh(i);
		}
	}
}

void Renderer::drawEntityDepth(const Entity& entity,
	const MeshComponent* meshComponent,
	const glm::mat4& lightSpaceMatrix) const {
	glm::mat4 model = entity.getTransform().getMatrix();
	depthShader->setMat4("uLightSpaceMatrix", lightSpaceMatrix);
	depthShader->setMat4("uModel", model);
    meshComponent->getMesh()->draw();
}
