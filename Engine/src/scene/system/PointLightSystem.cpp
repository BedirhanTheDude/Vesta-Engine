#include "PointLightSystem.h"

#include <persistance/Archive.h>

#include <glm/glm.hpp>

namespace PointLightSystem {

    void serialize(const PointLight& light, Archive& arch) {
        arch.set("color", light.color);
        arch.set("ambientStrength", light.ambientStrength);
        arch.set("diffuseStrength", light.diffuseStrength);
        arch.set("specularStrength", light.specularStrength);

        arch.set("constant", light.constant);
        arch.set("linear", light.linear);
        arch.set("quadratic", light.quadratic);
    }

    void deserialize(PointLight& light, const Archive& arch) {
        if (!arch.get("color", light.color)) light.color = glm::vec3(1.0f);
        if (!arch.get("ambientStrength", light.ambientStrength)) light.ambientStrength = 0.1f;
        if (!arch.get("diffuseStrength", light.diffuseStrength)) light.diffuseStrength = 1.0f;
        if (!arch.get("specularStrength", light.specularStrength)) light.specularStrength = 0.8f;

        if (!arch.get("constant", light.constant)) light.constant = 1.0f;
        if (!arch.get("linear", light.linear)) light.linear = 0.09f;
        if (!arch.get("quadratic", light.quadratic)) light.quadratic = 0.032f;
    }
}
