#pragma once

#include <glm/glm.hpp>
#include <memory>

class ShaderProgram;
class Scene;
class Entity;
class CameraComponent;
class MeshComponent;
class MaterialComponent;
class DirectionalLightComponent;
class PointLightComponent;

class Renderer {
	friend class Application;
public:
	~Renderer();
    
	void setClearColor(const glm::vec4& color) { clearColor = color; }
	void render(const Scene& scene);
	void render(const Scene& scene, const glm::mat4& viewOverride, const glm::mat4& projOverride);

private:
	void initShadowMap();

	void drawEntity(const Entity& entity,
		const MeshComponent* meshComponent,
		const MaterialComponent* materialComponent,
		const glm::mat4& view,
		const glm::mat4& proj,
		const glm::vec3& cameraPosition,
		const DirectionalLightComponent* dirLight,
		const std::vector<PointLightComponent*> pointLights,
		const glm::mat4& lightSpaceMatrix,
		bool transparentOnly = false) const;

	void drawEntityDepth(const Entity& entity,
						 const MeshComponent* meshComponent,
						 const glm::mat4& lightSpaceMatrix) const;

private:
	Renderer();

	glm::vec4 clearColor{ 0.1f, 0.1f, 0.1f, 1.0f };

	unsigned int shadowFBO = 0;
	unsigned int shadowDepthTexture = 0;
	mutable unsigned int lightSSBO = 0;
	static const int SHADOW_WIDTH  = 2048;
	static const int SHADOW_HEIGHT = 2048;

	std::unique_ptr<ShaderProgram> depthShader;
};
