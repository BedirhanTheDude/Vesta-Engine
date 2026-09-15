#pragma once

#include <scene/components/Component.h>

#include <glm/glm.hpp>

class CameraComponent;
class Archive;

class DirectionalLightComponent : public Component {
public:
    DirectionalLightComponent(float ambient = 0.2f, float diffuse = 1.0f, float specular = 0.5f,
        float shadowDistance = 30.0f, float shadowOrthoSize = 40.0f, float shadowNear = 0.1f, float shadowFar = 200.0f)
        : ambientStrength(ambient), diffuseStrength(diffuse), specularStrength(specular),
          shadowDistance(shadowDistance), shadowOrthoSize(shadowOrthoSize),
          shadowNear(shadowNear), shadowFar(shadowFar) {
        registerProperty("color", PropertyType::Color, &this->color);

        registerProperty("ambientStrength", PropertyType::Float, &this->ambientStrength);
        registerProperty("diffuseStrength", PropertyType::Float, &this->diffuseStrength);
        registerProperty("specularStrength", PropertyType::Float, &this->specularStrength);

        registerProperty("shadowDistance", PropertyType::Float, &this->shadowDistance);
        registerProperty("shadowOrthoSize", PropertyType::Float, &this->shadowOrthoSize);
        registerProperty("shadowNear", PropertyType::Float, &this->shadowNear);
        registerProperty("shadowFar", PropertyType::Float, &this->shadowFar);
    }

	glm::vec3 color = glm::vec3{ 1.0f, 1.0f, 1.0f };

	float ambientStrength;
	float diffuseStrength;
	float specularStrength;

    float shadowDistance;
	float shadowOrthoSize;
    float shadowNear;
    float shadowFar;

    glm::mat4 getLightSpaceMatrix(const glm::vec3& camPos) const;

    glm::vec3 getDirection() const;

    void serialize(Archive& arch) const override;
    void deserialize(const Archive& arch) override;
};