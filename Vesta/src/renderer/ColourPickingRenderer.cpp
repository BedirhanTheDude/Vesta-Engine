#include "Colourpickingrenderer.h"
#include <glad/glad.h>
#include <scene/Scene.h>
#include <scene/Entity.h>
#include <scene/components/Transform.h>
#include <scene/components/MeshData.h>
#include <scene/system/TransformSystem.h>
#include <ecs/ComponentPool.h>
#include <ecs/ComponentPoolRegistry.h>
#include <cstdio>

ColourPickingRenderer::ColourPickingRenderer() {
	compileShader();
}

ColourPickingRenderer::~ColourPickingRenderer() {
	destroyFramebuffer();
	if (pickingShader)
		glDeleteProgram(pickingShader);
}

void ColourPickingRenderer::resize(int width, int height) {
	if (width <= 0) width = 1;
	if (height <= 0) height = 1;
	if (width == viewportWidth && height == viewportHeight && fbo != 0)
		return;
	viewportWidth = width;
	viewportHeight = height;
	destroyFramebuffer();
	createFramebuffer(width, height);
}

void ColourPickingRenderer::createFramebuffer(int w, int h) {
	glGenTextures(1, &colourTexture);
	glBindTexture(GL_TEXTURE_2D, colourTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colourTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		std::fprintf(stderr, "colourpicking FBO error: %d\n", status);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ColourPickingRenderer::destroyFramebuffer() {
	if (fbo) { glDeleteFramebuffers(1, &fbo); fbo = 0; }
	if (colourTexture) { glDeleteTextures(1, &colourTexture); colourTexture = 0; }
	if (rbo) { glDeleteRenderbuffers(1, &rbo); rbo = 0; }
}

void ColourPickingRenderer::compileShader() {
	const char* vertexSrc = R"(
		#version 330 core
		layout(location = 0) in vec3 aPos;
		uniform mat4 uMVP;
		void main() {
			gl_Position = uMVP * vec4(aPos, 1.0);
		}
	)";
	const char* fragmentSrc = R"(
		#version 330 core
		out vec4 FragColor;
		uniform vec3 uColour;
		void main() {
			FragColor = vec4(uColour, 1.0);
		}
	)";

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
	glCompileShader(vertexShader);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
	glCompileShader(fragmentShader);

	pickingShader = glCreateProgram();
	glAttachShader(pickingShader, vertexShader);
	glAttachShader(pickingShader, fragmentShader);
	glLinkProgram(pickingShader);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void ColourPickingRenderer::buildEntityMap(const Scene* scene) {
	entityToId.clear();
	idToEntity.clear();
	if (!scene) return;
	uint32_t id = 1;
	for (const Entity& e : scene->getEntities()) {
		entityToId[e.getID()] = id;
		idToEntity[id] = e;
		++id;
	}
}

glm::vec3 ColourPickingRenderer::getPickedColour(const Entity& entity) const {
	auto it = entityToId.find(entity.getID());
	if (it == entityToId.end())
		return glm::vec3(0.0f);
	return idToColour(it->second);
}

void ColourPickingRenderer::renderPickingPass(
	const Scene* scene,
	const glm::mat4& view,
	const glm::mat4& projection,
	const DrawCallback& drawCallback
) {
	if (!fbo || !scene) return;

	buildEntityMap(scene);

	GLint previousFBO = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, viewportWidth, viewportHeight);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	// picking only reads the scene, the transforms' lazily cached (mutable) matrices are all it refreshes
	Scene* mutableScene = const_cast<Scene*>(scene);

	ECS::ComponentPoolRegistry* registry = scene->getComponentPoolRegistry();
	ECS::ComponentPool<Transform>& transforms = registry->getTransforms();

	for (auto item : registry->getMeshes()) {
		if (!item.component.mesh) continue;

		// entities created this frame are not flushed yet, so they are not in the map either
		auto it = entityToId.find(item.entity.entityId);
		if (it == entityToId.end()) continue;

		Transform* transform = transforms.get(item.entity);
		if (!transform) continue;

		glm::mat4 model = TransformSystem::getMatrix(*transform, mutableScene);
		glm::mat4 mvp = projection * view * model;

		drawCallback(pickingShader, idToEntity[it->second], mvp);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);
	glEnable(GL_BLEND);
}

void ColourPickingRenderer::beginPicking() {
	if (!fbo) return;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &savedFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, viewportWidth, viewportHeight);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
}

void ColourPickingRenderer::endPicking() {
	glBindFramebuffer(GL_FRAMEBUFFER, savedFBO);
	glEnable(GL_BLEND);
}

Entity ColourPickingRenderer::pickEntity(float mouseX, float mouseY) const {
	if (!fbo) return Entity();

	int x = static_cast<int>(mouseX);
	int y = viewportHeight - 1 - static_cast<int>(mouseY);

	if (x < 0 || x >= viewportWidth || y < 0 || y >= viewportHeight)
		return Entity();

	GLint previousFBO = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFinish();

	uint8_t pixel[3] = { 0, 0, 0 };
	glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

	glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);

	uint32_t id = colourtoId(pixel[0], pixel[1], pixel[2]);
	if (id == 0)
		return Entity();

	auto it = idToEntity.find(id);
	if (it == idToEntity.end())
		return Entity();
	return it->second;
}