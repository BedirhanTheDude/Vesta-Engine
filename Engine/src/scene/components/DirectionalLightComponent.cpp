#include <scene/components/DirectionalLightComponent.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <scene/Entity.h>

#include <scene/components/ComponentFactory.h>
#include <persistance/Archive.h>

REGISTER(DirectionalLightComponent);

glm::mat4 DirectionalLightComponent::getLightSpaceMatrix(const glm::vec3& camPos) const {
    glm::vec3 dir = glm::normalize(getDirection());
    glm::vec3 lightPos = camPos - dir * 20.0f;

    glm::mat4 lightView = glm::lookAt(lightPos, camPos, glm::vec3(0, 1, 0));
    glm::mat4 lightProj = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 100.0f);

    return lightProj * lightView;
}

glm::vec3 DirectionalLightComponent::getDirection() const {
    return owner->transform.forward();
}

void DirectionalLightComponent::serialize(Archive& arch) const {
    arch.set("color", color);
    arch.set("ambientStrength", ambientStrength);
    arch.set("diffuseStrength", diffuseStrength);
    arch.set("specularStrength", specularStrength);

    arch.set("shadowDistance", shadowDistance);
    arch.set("shadowOrthoSize", shadowOrthoSize);
    arch.set("shadowNear", shadowNear);
    arch.set("shadowFar", shadowFar);
}

void DirectionalLightComponent::deserialize(const Archive& arch) {
    if (!arch.get("color", color)) color = glm::vec3(1.0f);
    if (!arch.get("ambientStrength", ambientStrength)) ambientStrength = 0.2f;
    if (!arch.get("diffuseStrength", diffuseStrength)) diffuseStrength = 1.0f;
    if (!arch.get("specularStrength", specularStrength)) specularStrength = 0.5f;

    if (!arch.get("shadowDistance", shadowDistance)) shadowDistance = 30.0f;
    if (!arch.get("shadowOrthoSize", shadowOrthoSize)) shadowOrthoSize = 40.0f;
    if (!arch.get("shadowNear", shadowNear)) shadowNear = 0.1f;
    if (!arch.get("shadowFar", shadowFar)) shadowFar = 200.0f;
}