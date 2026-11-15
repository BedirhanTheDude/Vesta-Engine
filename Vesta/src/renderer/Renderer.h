#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <memory>
#include <vector>

class ShaderProgram;
class Scene;

struct MeshData;
struct MaterialData;

struct PointLightGPU;	// defined in Renderer.cpp
struct FrameData;		// this too
struct TransparentDraw; // this too...

namespace RenderingSystem {
	struct SceneRenderView;
}

struct ShadowParams {
	glm::mat4 lightSpaceMatrix;
	float texelWorldSize; // world units covered by one shadow map texel
	float depthRange;     // far - near of the shadow projection, world units
};

class Renderer {
	friend class Application;
public:
	~Renderer();

	void setClearColor(const glm::vec4& color) { clearColor = color; }
	void render(const Scene& scene);
	void render(const Scene& scene, const glm::mat4& viewOverride, const glm::mat4& projOverride);

private:
	void render(const RenderingSystem::SceneRenderView* renderView,
		const glm::mat4& view, const glm::mat4& proj);

	void initShadowMap();

	// uploads point light data to GPU memory
	void uploadPointLights(const std::vector<PointLightGPU>& pointLights) const;

	void drawEntity(const glm::mat4& model,
		const MeshData& mesh,
		const MaterialData& materials,
		FrameData& frame,
		bool transparentOnly = false) const;

	void drawEntityDepth(const glm::mat4& model, const MeshData& mesh) const;

private:
	Renderer();

	glm::vec4 clearColor{ 0.1f, 0.1f, 0.1f, 1.0f };

	unsigned int shadowFBO = 0;
	unsigned int shadowDepthTexture = 0;
	mutable unsigned int lightSSBO = 0;
	static const int SHADOW_WIDTH  = 2048;
	static const int SHADOW_HEIGHT = 2048;

	std::unique_ptr<ShaderProgram> depthShader;

	uint64_t frameEpoch = 0; // incremented per render(), see ShaderProgram::beginEpoch

	// scratch of render(), kept between calls so their capacity is reused instead of allocating every frame
	std::vector<PointLightGPU> pointLightScratch;
	std::vector<TransparentDraw> transparentDrawScratch;
};
