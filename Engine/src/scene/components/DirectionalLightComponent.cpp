#include <scene/components/DirectionalLightComponent.h>

#include <glm/glm.hpp>

#include <scene/Entity.h>

#include <scene/components/DirectionalLight.h>
#include <scene/components/Transform.h>
#include <scene/components/ProxyUtil.h>
#include <scene/system/DirectionalLightSystem.h>

#include <scene/components/ComponentFactory.h>
#include <persistance/Archive.h>

REGISTER(DirectionalLightComponent);

void DirectionalLightComponent::init(float ambient, float diffuse, float specular,
    float shadowDistance, float shadowOrthoSize, float shadowNear, float shadowFar) {
    DirectionalLight* light = resolveComponent<DirectionalLight>(entity);
    if (!light) return;

    light->ambientStrength = ambient;
    light->diffuseStrength = diffuse;
    light->specularStrength = specular;

    light->shadowDistance = shadowDistance;
    light->shadowOrthoSize = shadowOrthoSize;
    light->shadowNear = shadowNear;
    light->shadowFar = shadowFar;
}

glm::vec3 DirectionalLightComponent::getColor() const {
    DirectionalLight* light = resolveComponent<DirectionalLight>(entity);
    return light ? light->color : glm::vec3(1.0f);
}

void DirectionalLightComponent::setColor(const glm::vec3& color) {
    if (DirectionalLight* light = resolveComponent<DirectionalLight>(entity)) light->color = color;
}

float DirectionalLightComponent::getAmbientStrength() const {
    DirectionalLight* light = resolveComponent<DirectionalLight>(entity);
    return light ? light->ambientStrength : 0.2f;
}

void DirectionalLightComponent::setAmbientStrength(float ambient) {
    if (DirectionalLight* light = resolveComponent<DirectionalLight>(entity)) light->ambientStrength = ambient;
}

float DirectionalLightComponent::getDiffuseStrength() const {
    DirectionalLight* light = resolveComponent<DirectionalLight>(entity);
    return light ? light->diffuseStrength : 1.0f;
}

void DirectionalLightComponent::setDiffuseStrength(float diffuse) {
    if (DirectionalLight* light = resolveComponent<DirectionalLight>(entity)) light->diffuseStrength = diffuse;
}

float DirectionalLightComponent::getSpecularStrength() const {
    DirectionalLight* light = resolveComponent<DirectionalLight>(entity);
    return light ? light->specularStrength : 0.5f;
}

void DirectionalLightComponent::setSpecularStrength(float specular) {
    if (DirectionalLight* light = resolveComponent<DirectionalLight>(entity)) light->specularStrength = specular;
}

float DirectionalLightComponent::getShadowDistance() const {
    DirectionalLight* light = resolveComponent<DirectionalLight>(entity);
    return light ? light->shadowDistance : 30.0f;
}

void DirectionalLightComponent::setShadowDistance(float shadowDistance) {
    if (DirectionalLight* light = resolveComponent<DirectionalLight>(entity)) light->shadowDistance = shadowDistance;
}

float DirectionalLightComponent::getShadowOrthoSize() const {
    DirectionalLight* light = resolveComponent<DirectionalLight>(entity);
    return light ? light->shadowOrthoSize : 40.0f;
}

void DirectionalLightComponent::setShadowOrthoSize(float shadowOrthoSize) {
    if (DirectionalLight* light = resolveComponent<DirectionalLight>(entity)) light->shadowOrthoSize = shadowOrthoSize;
}

float DirectionalLightComponent::getShadowNear() const {
    DirectionalLight* light = resolveComponent<DirectionalLight>(entity);
    return light ? light->shadowNear : 0.1f;
}

void DirectionalLightComponent::setShadowNear(float shadowNear) {
    if (DirectionalLight* light = resolveComponent<DirectionalLight>(entity)) light->shadowNear = shadowNear;
}

float DirectionalLightComponent::getShadowFar() const {
    DirectionalLight* light = resolveComponent<DirectionalLight>(entity);
    return light ? light->shadowFar : 200.0f;
}

void DirectionalLightComponent::setShadowFar(float shadowFar) {
    if (DirectionalLight* light = resolveComponent<DirectionalLight>(entity)) light->shadowFar = shadowFar;
}

glm::mat4 DirectionalLightComponent::getLightSpaceMatrix(const glm::vec3& camPos, const glm::vec3& camForward) const {
    DirectionalLight* light = resolveComponent<DirectionalLight>(entity);
    Transform* transform = resolveComponent<Transform>(entity);
    if (!light || !transform) return glm::mat4(1.0f);

    return DirectionalLightSystem::getLightSpaceMatrix(*light, *transform, camPos, camForward);
}

glm::vec3 DirectionalLightComponent::getDirection() const {
    Transform* transform = resolveComponent<Transform>(entity);
    return transform ? DirectionalLightSystem::getDirection(*transform) : glm::vec3(0.0f, 0.0f, -1.0f);
}

void DirectionalLightComponent::serialize(Archive& arch) const {
    if (DirectionalLight* light = resolveComponent<DirectionalLight>(entity))
        DirectionalLightSystem::serialize(*light, arch);
}

void DirectionalLightComponent::deserialize(const Archive& arch) {
    if (DirectionalLight* light = resolveComponent<DirectionalLight>(entity))
        DirectionalLightSystem::deserialize(*light, arch);
}
