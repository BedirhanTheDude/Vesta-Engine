#include "Renderer.h"

#include <memory>
#include <vector>
#include <algorithm>

#include <scene/Scene.h>
#include <scene/Entity.h>

#include <scene/components/Transform.h>
#include <scene/components/MeshData.h>
#include <scene/components/MaterialData.h>

#include <scene/system/RenderingSystem.h>

#include <renderer/ShaderProgram.h>
#include <renderer/Mesh.h>
#include <renderer/Material.h>

#include <glad/glad.h>

#include <cstdint>


// DO NOT change order of this struct pls, memory alignment may break
struct PointLightGPU {
	glm::vec3 position;
	glm::vec3 color;
	float ambientIntensity;
	float diffuseIntensity;
	float specularIntensity;
	float constant;
	float linear;
	float quadratic;
};

static bool hasAnyTransparent(const MaterialData& materials) {
	for (const auto& material : materials.materials) {
		if (material && material->transparent) return true;
	}
	return false;
}

static Material* getMaterial(const MaterialData& materials, int index) {
	if (index < 0 || index >= static_cast<int>(materials.materials.size()))
		return nullptr;
	return materials.materials[index].get();
}

struct FrameData {
	glm::mat4 view;
	glm::mat4 proj;
	glm::vec3 cameraPosition;

	const RenderingSystem::DirectionalLightData* dirLightData;

	uint32_t pointLightCount;
	ShadowParams shadow;

	uint64_t epoch; // identifies this render() call, a program that already saw it has the values above

	// what the draw calls have already set up, so they can skip redundant work
	unsigned int currentProgram = 0;               // program bound by the last draw
	const Material* lastMaterial = nullptr;        // material applied by the last draw
};

struct TransparentDraw {
	float distance;
	const MeshData* mesh;
	const MaterialData* materials;
	const glm::mat4* model;
};

static const int SHADOW_TEXTURE_UNIT = 4;

static void applyFrameUniforms(const ShaderProgram& shader, const FrameData& frame) {
	shader.setMat4("uView", frame.view);
	shader.setMat4("uProj", frame.proj);
	shader.setVec3("uViewPos", frame.cameraPosition);

	shader.setMat4("uLightSpaceMatrix", frame.shadow.lightSpaceMatrix);
	shader.setFloat("uShadowTexelSize", frame.shadow.texelWorldSize);
	shader.setFloat("uShadowDepthRange", frame.shadow.depthRange);
	shader.setInt("uShadowMap", SHADOW_TEXTURE_UNIT);

	if (frame.dirLightData->hasDirLight) {
		shader.setBool("uHasDirectionalLight", true);
		shader.setVec3("uDirectionalLight.direction", frame.dirLightData->direction);
		shader.setVec3("uDirectionalLight.color", frame.dirLightData->color);
		shader.setFloat("uDirectionalLight.ambientIntensity", frame.dirLightData->ambientStrength);
		shader.setFloat("uDirectionalLight.diffuseIntensity", frame.dirLightData->diffuseStrength);
		shader.setFloat("uDirectionalLight.specularIntensity", frame.dirLightData->specularStrength);
	}
	else {
		shader.setBool("uHasDirectionalLight", false);
	}

	shader.setInt("uPointLightCount", frame.pointLightCount);
}

Renderer::Renderer() {
	initShadowMap();

	depthShader = std::make_unique<ShaderProgram>("depth", ShaderProgram::getEngineShaderDirectory() / "depth");
}

Renderer::~Renderer() {
	if (shadowFBO) glDeleteFramebuffers(1, &shadowFBO);
	if (shadowDepthTexture) glDeleteTextures(1, &shadowDepthTexture);
	if (lightSSBO) glDeleteBuffers(1, &lightSSBO);
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
	const RenderingSystem::SceneRenderView* renderView = RenderingSystem::getSceneRenderView(scene);
	glm::mat4 view = RenderingSystem::getActiveCameraView(scene);
	glm::mat4 proj = RenderingSystem::getActiveCameraProj(scene);
	
	render(renderView, view, proj);
}

void Renderer::render(const Scene& scene, const glm::mat4& view, const glm::mat4& proj) {
	const RenderingSystem::SceneRenderView* renderView = RenderingSystem::getSceneRenderView(scene);
	render(renderView, view, proj);
}

void Renderer::render(const RenderingSystem::SceneRenderView* renderView,
	const glm::mat4& view, const glm::mat4& proj) {
	if (!renderView->isRenderable) return;

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	GLint previousFBO;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFBO);

	glm::mat4 inverseView = glm::inverse(view);
	glm::vec3 camPos = glm::vec3(inverseView[3]);
	glm::vec3 camForward = -glm::normalize(glm::vec3(inverseView[2]));

	// the members keep their capacity between frames, nothing is allocated here in the steady state
	std::vector<PointLightGPU>& pointLights = pointLightScratch;
	pointLights.clear();

	for (auto item : renderView->pointLightData) {
		PointLightGPU pointLight;
		pointLight.position = item.position;
		pointLight.ambientIntensity = item.ambientIntensity;
		pointLight.color = item.color;
		pointLight.diffuseIntensity = item.diffuseIntensity;
		pointLight.specularIntensity = item.specularIntensity;
		pointLight.constant = item.constant;
		pointLight.linear = item.linear;
		pointLight.quadratic = item.quadratic;

		pointLights.push_back(pointLight);
	}

	const RenderingSystem::DirectionalLightData& dirLightData = renderView->dirLightData;

	ShadowParams shadow{ glm::mat4(1.0f), 1.0f, 1.0f };

	if (dirLightData.hasDirLight) {
		const auto frustum = renderView->shadowData.frustum;

		shadow.lightSpaceMatrix = std::move(renderView->shadowData.lightSpaceMatrix);
		shadow.texelWorldSize = frustum.size / static_cast<float>(SHADOW_WIDTH);
		shadow.depthRange = frustum.farPlane - frustum.nearPlane;

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		depthShader->use();
		depthShader->setMat4("uLightSpaceMatrix", shadow.lightSpaceMatrix);

		for (uint32_t i = 0; i < renderView->entityCount; ++i) {
			drawEntityDepth(renderView->models.at(i), renderView->meshData.at(i));
		}

		glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);
	}

	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	glEnable(GL_DEPTH_TEST);
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	std::vector<TransparentDraw>& transparentDraws = transparentDrawScratch;
	transparentDraws.clear();

	glEnable(GL_CULL_FACE);

	FrameData frame;
	frame.epoch = ++frameEpoch;
	frame.view = view;
	frame.proj = proj;
	frame.cameraPosition = camPos;
	frame.dirLightData = &renderView->dirLightData;
	frame.pointLightCount = static_cast<int>(pointLights.size());
	frame.shadow = shadow;

	// once for the whole frame: the lights and the shadow map are the same for every draw call below
	uploadPointLights(pointLights);

	glActiveTexture(GL_TEXTURE0 + SHADOW_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D, shadowDepthTexture);
	glActiveTexture(GL_TEXTURE0);

	for (uint32_t i = 0; i < renderView->entityCount; ++i) {
		const MeshData& meshData = renderView->meshData.at(i);
		const MaterialData& materialData = renderView->materialData.at(i);
		const glm::mat4& modelMatrix = renderView->models.at(i);

		drawEntity(modelMatrix, meshData, materialData, frame, false);

		if (hasAnyTransparent(materialData)) {
			glm::vec3 worldPosition = modelMatrix[3];
			transparentDraws.push_back({ glm::length(worldPosition - camPos), &meshData, &materialData, &modelMatrix });
		}
	}

	// farthest first
	std::sort(transparentDraws.begin(), transparentDraws.end(),
		[](const TransparentDraw& a, const TransparentDraw& b) { return a.distance > b.distance; });

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	for (const TransparentDraw& draw : transparentDraws) {
		drawEntity(*draw.model, *draw.mesh, *draw.materials, frame, true);
	}

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void Renderer::uploadPointLights(const std::vector<PointLightGPU>& pointLights) const {
	if (pointLights.empty()) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		return;
	}

	if (lightSSBO == 0)
		glGenBuffers(1, &lightSSBO);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
		pointLights.size() * sizeof(PointLightGPU),
		pointLights.data(),
		GL_DYNAMIC_DRAW);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO);
}

void Renderer::drawEntity(const glm::mat4& model,
	const MeshData& mesh,
	const MaterialData& materials,
	FrameData& frame,
	bool transparentOnly) const {

	if (!mesh.mesh) return;

	auto drawWithMaterial = [&](Material& material) {
		ShaderProgram& shader = *material.shader;
		unsigned int programID = shader.getID();

		if (frame.currentProgram != programID) {
			shader.use();
			frame.currentProgram = programID;
		}

		// first time this program is used in this render() call: give it the frame's uniforms
		if (shader.beginEpoch(frame.epoch))
			applyFrameUniforms(shader, frame);

		// consecutive draws with the same material
		// keep the uniforms and the diffuse texture binding the previous draw set
		if (frame.lastMaterial != &material) {
			material.apply();
			frame.lastMaterial = &material;
		}

		shader.setMat4("uModel", model);
	};

	const auto& submeshes = mesh.mesh->getSubMeshes();

	if (submeshes.empty()) {
		Material* mat = getMaterial(materials, 0);
		if (!mat || !mat->shader) return;
		if (mat->transparent != transparentOnly) return; // skip wrong pass
		drawWithMaterial(*mat);
		mesh.mesh->draw();
	}
	else {
		for (int i = 0; i < static_cast<int>(submeshes.size()); i++) {
			Material* mat = getMaterial(materials, submeshes[i].materialIndex);
			if (!mat) mat = getMaterial(materials, 0);
			if (!mat || !mat->shader) continue;
			if (mat->transparent != transparentOnly) continue; // skip wrong pass
			drawWithMaterial(*mat);
			mesh.mesh->drawSubMesh(i);
		}
	}
}

void Renderer::drawEntityDepth(const glm::mat4& model, const MeshData& mesh) const {
	depthShader->setMat4("uModel", model);
	mesh.mesh->draw();
}
