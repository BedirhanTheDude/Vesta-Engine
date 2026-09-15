#include <scene/components/PointLightComponent.h>

#include <scene/components/ComponentFactory.h>
#include <persistance/Archive.h>

#include <glm/glm.hpp>

REGISTER(PointLightComponent);

void PointLightComponent::serialize(Archive& arch) const {
    arch.set("color", color);
    arch.set("ambientStrength", ambientStrength);
    arch.set("diffuseStrength", diffuseStrength);
    arch.set("specularStrength", specularStrength);

    arch.set("constant", constant);
    arch.set("linear", linear);
    arch.set("quadratic", quadratic);
}

void PointLightComponent::deserialize(const Archive& arch) {
    if (!arch.get("color", color)) color = glm::vec3(1.0f);
    if (!arch.get("ambientStrength", ambientStrength)) ambientStrength = 0.1f;
    if (!arch.get("diffuseStrength", diffuseStrength)) diffuseStrength = 1.0f;
    if (!arch.get("specularStrength", specularStrength)) specularStrength = 0.8f;

    if (!arch.get("constant", constant)) constant = 1.0f;
    if (!arch.get("linear", linear)) linear = 0.09f;
    if (!arch.get("quadratic", quadratic)) quadratic = 0.032f;
}