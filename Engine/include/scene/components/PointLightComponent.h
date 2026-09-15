#pragma once

#include <scene/components/Component.h>

#include <glm/glm.hpp>

class Archive;

class PointLightComponent : public Component {
public:
    PointLightComponent(float ambient = 0.1f, float diffuse = 1.0f, float specular = 0.8f,
        float constant = 1.0f, float linear = 0.09f, float quadratic = 0.032f)
        : ambientStrength(ambient), diffuseStrength(diffuse), specularStrength(specular),
          constant(constant), linear(linear), quadratic(quadratic) {
        registerProperty("Color", PropertyType::Color, &this->color);

        registerProperty("Ambient Strength", PropertyType::Float, &this->ambientStrength);
        registerProperty("Diffuse Strength", PropertyType::Float, &this->diffuseStrength);
        registerProperty("Specular Strength", PropertyType::Float, &this->specularStrength);

        registerProperty("Constant", PropertyType::Float, &this->constant);
        registerProperty("Linear", PropertyType::Float, &this->linear);
        registerProperty("Quadratic", PropertyType::Float, &this->quadratic);
    }

    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

    float ambientStrength = 0.1f;
    float diffuseStrength = 1.0f;
    float specularStrength = 0.8f;

    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    void serialize(Archive& arch) const;
    void deserialize(const Archive& arch);
};