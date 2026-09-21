#include "DirectionalLightSystem.h"

#include <scene/system/TransformSystem.h>

#include <persistance/Archive.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace DirectionalLightSystem {

    glm::vec3 getDirection(const Transform& transform) {
        return TransformSystem::forward(transform);
    }

    // the inspector can set anything, keep the projection well formed instead of dividing by zero
    ShadowFrustum getShadowFrustum(const DirectionalLight& light) {
        ShadowFrustum frustum;
        frustum.size = std::max(light.shadowOrthoSize, 0.001f);
        frustum.nearPlane = std::max(light.shadowNear, 0.001f);
        frustum.farPlane = std::max(light.shadowFar, frustum.nearPlane + 0.001f);

        return frustum;
    }

    glm::mat4 getLightSpaceMatrix(const DirectionalLight& light, const Transform& transform,
        const glm::vec3& camPos, const glm::vec3& camForward) {
        ShadowFrustum frustum = getShadowFrustum(light);
        float halfSize = frustum.size * 0.5f;

        glm::vec3 dir = glm::normalize(getDirection(transform));

        glm::vec3 lookPerpendicular = camForward - dir * glm::dot(camForward, dir);
        glm::vec3 center = camPos + lookPerpendicular * (halfSize * 0.5f);

        glm::vec3 lightPos = center - dir * light.shadowDistance;

        // looking straight up or down is parallel to the usual up vector, which makes lookAt produce NaNs
        glm::vec3 up = std::abs(glm::dot(dir, glm::vec3(0, 1, 0))) > 0.999f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);

        glm::mat4 lightView = glm::lookAt(lightPos, center, up);
        glm::mat4 lightProj = glm::ortho(-halfSize, halfSize, -halfSize, halfSize, frustum.nearPlane, frustum.farPlane);

        return lightProj * lightView;
    }

    void serialize(const DirectionalLight& light, Archive& arch) {
        arch.set("color", light.color);
        arch.set("ambientStrength", light.ambientStrength);
        arch.set("diffuseStrength", light.diffuseStrength);
        arch.set("specularStrength", light.specularStrength);

        arch.set("shadowDistance", light.shadowDistance);
        arch.set("shadowOrthoSize", light.shadowOrthoSize);
        arch.set("shadowNear", light.shadowNear);
        arch.set("shadowFar", light.shadowFar);
    }

    void deserialize(DirectionalLight& light, const Archive& arch) {
        if (!arch.get("color", light.color)) light.color = glm::vec3(1.0f);
        if (!arch.get("ambientStrength", light.ambientStrength)) light.ambientStrength = 0.2f;
        if (!arch.get("diffuseStrength", light.diffuseStrength)) light.diffuseStrength = 1.0f;
        if (!arch.get("specularStrength", light.specularStrength)) light.specularStrength = 0.5f;

        if (!arch.get("shadowDistance", light.shadowDistance)) light.shadowDistance = 30.0f;
        if (!arch.get("shadowOrthoSize", light.shadowOrthoSize)) light.shadowOrthoSize = 40.0f;
        if (!arch.get("shadowNear", light.shadowNear)) light.shadowNear = 0.1f;
        if (!arch.get("shadowFar", light.shadowFar)) light.shadowFar = 200.0f;
    }
}
