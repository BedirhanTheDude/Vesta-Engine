#pragma once

#include <glm/glm.hpp>
#include <memory>

class ShaderProgram;
class Texture;

class Material {
public:
	Material(std::shared_ptr<ShaderProgram> shader = nullptr,
		glm::vec3 albedo = glm::vec3{ 0.7f, 0.7f, 0.7f },
		float ambientReflectance = 1.0f,
		float specularReflectance = 0.2,
		float shininess = 32.0f) :
		shader(shader), albedo(albedo),
		ambientReflectance(ambientReflectance),
		specularReflectance(specularReflectance),
		shininess(shininess) {}

	std::shared_ptr<Texture> diffuseTexture = nullptr;
	std::shared_ptr<ShaderProgram> shader;

	glm::vec3 albedo = glm::vec3{ 0.7f, 0.7f, 0.7f };
	glm::vec3 emission = glm::vec3(0.0f);

	float ambientReflectance = 1.0f;
	float specularReflectance = 0.2f;
	float shininess = 32.0f;

	float alpha = 1.0f;
	bool transparent = false;

	void apply() const;
};