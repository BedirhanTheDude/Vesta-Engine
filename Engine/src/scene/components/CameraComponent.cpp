#include <scene/components/CameraComponent.h>

#include <scene/Scene.h>
#include <scene/Entity.h>

#include <scene/components/Camera.h>
#include <scene/components/Transform.h>
#include <scene/components/ProxyUtil.h>
#include <scene/system/CameraSystem.h>

#include <scene/components/ComponentFactory.h>
#include <persistance/Archive.h>

REGISTER(CameraComponent);

void CameraComponent::init(float fov, float aspect, float nearPlane, float farPlane) {
    Camera* camera = resolveComponent<Camera>(entity);
    if (!camera) return;

    camera->fov = fov;
    camera->aspect = aspect;
    camera->nearPlane = nearPlane;
    camera->farPlane = farPlane;
}

float CameraComponent::getFov() const {
    Camera* camera = resolveComponent<Camera>(entity);
    return camera ? camera->fov : 45.0f;
}

void CameraComponent::setFov(float fov) {
    if (Camera* camera = resolveComponent<Camera>(entity)) camera->fov = fov;
}

float CameraComponent::getAspect() const {
    Camera* camera = resolveComponent<Camera>(entity);
    return camera ? camera->aspect : 1.0f;
}

void CameraComponent::setAspect(float aspect) {
    if (Camera* camera = resolveComponent<Camera>(entity)) camera->aspect = aspect;
}

float CameraComponent::getNearPlane() const {
    Camera* camera = resolveComponent<Camera>(entity);
    return camera ? camera->nearPlane : 0.1f;
}

void CameraComponent::setNearPlane(float nearPlane) {
    if (Camera* camera = resolveComponent<Camera>(entity)) camera->nearPlane = nearPlane;
}

float CameraComponent::getFarPlane() const {
    Camera* camera = resolveComponent<Camera>(entity);
    return camera ? camera->farPlane : 100.0f;
}

void CameraComponent::setFarPlane(float farPlane) {
    if (Camera* camera = resolveComponent<Camera>(entity)) camera->farPlane = farPlane;
}

glm::mat4 CameraComponent::getViewMatrix() const {
    Transform* transform = resolveComponent<Transform>(entity);
    if (!transform) return glm::mat4(1.0f);

    return CameraSystem::getViewMatrix(*transform, sceneOf(entity));
}

glm::mat4 CameraComponent::getProjectionMatrix() const {
    Camera* camera = resolveComponent<Camera>(entity);
    return camera ? CameraSystem::getProjectionMatrix(*camera) : glm::mat4(1.0f);
}

bool CameraComponent::onAttach() {
    if (!entity.isValid()) return false;

    entity.getScene().onCameraAdded(entity);
    return true;
}

void CameraComponent::onDetach() {
    if (!entity.isValid()) return;

    if (entity.getScene().getActiveCameraEntity() == entity)
        entity.getScene().setActiveCamera(Entity());
}

void CameraComponent::serialize(Archive& arch) const {
    if (Camera* camera = resolveComponent<Camera>(entity))
        CameraSystem::serialize(*camera, arch);
}

void CameraComponent::deserialize(const Archive& arch) {
    if (Camera* camera = resolveComponent<Camera>(entity))
        CameraSystem::deserialize(*camera, arch);
}
