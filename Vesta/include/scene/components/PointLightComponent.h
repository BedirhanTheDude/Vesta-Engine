#pragma once

#include <scene/components/Component.h>

#include <glm/glm.hpp>

class Archive;

class PointLightComponent : public Component {
public:
    explicit PointLightComponent(const Entity& entity) : Component(entity) {}

    // what addComponent<PointLightComponent>(ambient, diffuse, ...) forwards to
    void init(float ambient = 0.1f, float diffuse = 1.0f, float specular = 0.8f,
        float constant = 1.0f, float linear = 0.09f, float quadratic = 0.032f);

    glm::vec3 getColor() const;
    void setColor(const glm::vec3& color);

    float getAmbientStrength() const;
    void setAmbientStrength(float ambient);

    float getDiffuseStrength() const;
    void setDiffuseStrength(float diffuse);

    float getSpecularStrength() const;
    void setSpecularStrength(float specular);

    float getConstant() const;
    void setConstant(float constant);

    float getLinear() const;
    void setLinear(float linear);

    float getQuadratic() const;
    void setQuadratic(float quadratic);

    void serialize(Archive& arch) const override;
    void deserialize(const Archive& arch) override;

    bool onAttach() { return true; }
    void onDetach() {}
};
