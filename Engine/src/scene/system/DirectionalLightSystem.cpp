#include "DirectionalLightSystem.h"

#include <persistance/Archive.h>

namespace DirectionalLightSystem {

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
