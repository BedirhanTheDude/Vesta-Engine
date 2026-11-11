#include <scene/components/PointLightComponent.h>

#include <scene/components/PointLight.h>
#include <scene/components/ProxyUtil.h>
#include <scene/system/PointLightSystem.h>

#include <scene/components/ComponentFactory.h>
#include <persistance/Archive.h>

#include <glm/glm.hpp>

REGISTER(PointLightComponent);

void PointLightComponent::init(float ambient, float diffuse, float specular,
    float constant, float linear, float quadratic) {
    PointLight* light = resolveComponent<PointLight>(entity);
    if (!light) return;

    light->ambientStrength = ambient;
    light->diffuseStrength = diffuse;
    light->specularStrength = specular;

    light->constant = constant;
    light->linear = linear;
    light->quadratic = quadratic;
}

glm::vec3 PointLightComponent::getColor() const {
    PointLight* light = resolveComponent<PointLight>(entity);
    return light ? light->color : glm::vec3(1.0f);
}

void PointLightComponent::setColor(const glm::vec3& color) {
    if (PointLight* light = resolveComponent<PointLight>(entity)) light->color = color;
}

float PointLightComponent::getAmbientStrength() const {
    PointLight* light = resolveComponent<PointLight>(entity);
    return light ? light->ambientStrength : 0.1f;
}

void PointLightComponent::setAmbientStrength(float ambient) {
    if (PointLight* light = resolveComponent<PointLight>(entity)) light->ambientStrength = ambient;
}

float PointLightComponent::getDiffuseStrength() const {
    PointLight* light = resolveComponent<PointLight>(entity);
    return light ? light->diffuseStrength : 1.0f;
}

void PointLightComponent::setDiffuseStrength(float diffuse) {
    if (PointLight* light = resolveComponent<PointLight>(entity)) light->diffuseStrength = diffuse;
}

float PointLightComponent::getSpecularStrength() const {
    PointLight* light = resolveComponent<PointLight>(entity);
    return light ? light->specularStrength : 0.8f;
}

void PointLightComponent::setSpecularStrength(float specular) {
    if (PointLight* light = resolveComponent<PointLight>(entity)) light->specularStrength = specular;
}

float PointLightComponent::getConstant() const {
    PointLight* light = resolveComponent<PointLight>(entity);
    return light ? light->constant : 1.0f;
}

void PointLightComponent::setConstant(float constant) {
    if (PointLight* light = resolveComponent<PointLight>(entity)) light->constant = constant;
}

float PointLightComponent::getLinear() const {
    PointLight* light = resolveComponent<PointLight>(entity);
    return light ? light->linear : 0.09f;
}

void PointLightComponent::setLinear(float linear) {
    if (PointLight* light = resolveComponent<PointLight>(entity)) light->linear = linear;
}

float PointLightComponent::getQuadratic() const {
    PointLight* light = resolveComponent<PointLight>(entity);
    return light ? light->quadratic : 0.032f;
}

void PointLightComponent::setQuadratic(float quadratic) {
    if (PointLight* light = resolveComponent<PointLight>(entity)) light->quadratic = quadratic;
}

void PointLightComponent::serialize(Archive& arch) const {
    if (PointLight* light = resolveComponent<PointLight>(entity))
        PointLightSystem::serialize(*light, arch);
}

void PointLightComponent::deserialize(const Archive& arch) {
    if (PointLight* light = resolveComponent<PointLight>(entity))
        PointLightSystem::deserialize(*light, arch);
}
