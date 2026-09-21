#pragma once

#include <scene/components/Component.h>

#include <glm/glm.hpp>

class Archive;

class DirectionalLightComponent : public Component {
public:
    explicit DirectionalLightComponent(const Entity& entity) : Component(entity) {}

    // what addComponent<DirectionalLightComponent>(ambient, diffuse, ...) forwards to
    void init(float ambient = 0.2f, float diffuse = 1.0f, float specular = 0.5f,
        float shadowDistance = 30.0f, float shadowOrthoSize = 40.0f, float shadowNear = 0.1f, float shadowFar = 200.0f);

    glm::vec3 getColor() const;
    void setColor(const glm::vec3& color);

    float getAmbientStrength() const;
    void setAmbientStrength(float ambient);

    float getDiffuseStrength() const;
    void setDiffuseStrength(float diffuse);

    float getSpecularStrength() const;
    void setSpecularStrength(float specular);

    float getShadowDistance() const;
    void setShadowDistance(float shadowDistance);

    float getShadowOrthoSize() const;
    void setShadowOrthoSize(float shadowOrthoSize);

    float getShadowNear() const;
    void setShadowNear(float shadowNear);

    float getShadowFar() const;
    void setShadowFar(float shadowFar);

    // camForward (normalized) shifts the shadow box towards where the camera looks, see DirectionalLightSystem
    glm::mat4 getLightSpaceMatrix(const glm::vec3& camPos, const glm::vec3& camForward = glm::vec3(0.0f)) const;

    glm::vec3 getDirection() const;

    void serialize(Archive& arch) const;
    void deserialize(const Archive& arch);

    bool onAttach() { return true; }
    void onDetach() {}
};
