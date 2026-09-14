#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <functional>
#include <cstdint>

class Entity;
class Scene;

class ColourPickingRenderer {
public:
	ColourPickingRenderer();
	~ColourPickingRenderer();

	void resize(int width, int height);
	void beginPicking();
	void endPicking();

	using DrawCallback = std::function<void(unsigned int shader, Entity* entity, const glm::mat4& mvp)>;

	void renderPickingPass(
		const Scene* scene,
		const glm::mat4& view,
		const glm::mat4& projection,
		const DrawCallback& drawCallback
	);

	Entity* pickEntity(float mouseX, float mouseY) const;
	glm::vec3 getPickedColour(Entity* entity) const;
	unsigned int getPickingShader() const { return pickingShader; }

private:
	unsigned int fbo = 0;
	unsigned int colourTexture = 0;
	unsigned int rbo = 0;
	unsigned int pickingShader = 0;

	int viewportWidth = 1;
	int viewportHeight = 1;
	int savedFBO = 0;

	std::unordered_map<Entity*, uint32_t> entityToId;
	std::unordered_map<uint32_t, Entity*> idToEntity;
	uint32_t nextId = 1;

	void createFramebuffer(int w, int h);
	void destroyFramebuffer();
	void compileShader();
	void buildEntityMap(const Scene* scene);

	static uint32_t colourtoId(uint8_t r, uint8_t g, uint8_t b){
		return (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
	}
	static glm::vec3 idToColour(uint32_t id) {
		uint8_t r = (id >> 16) & 0xFF;
		uint8_t g = (id >> 8) & 0xFF;
		uint8_t b = id & 0xFF;
		return glm::vec3(r, g, b) / 255.0f;
	}
};