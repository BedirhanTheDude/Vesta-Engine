#include <scene/components/CameraComponent.h>

#include <scene/Scene.h>
#include <scene/Entity.h>
#include <scene/components/TransformComponent.h>

#include <scene/components/ComponentFactory.h>
#include <persistance/Archive.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

REGISTER(CameraComponent);

glm::mat4 CameraComponent::getViewMatrix() const {
    glm::mat4 world = owner->getTransform().getMatrix();

    glm::vec3 pos = glm::vec3(world[3]);
    glm::vec3 forward = glm::normalize(glm::vec3(world[2]));
    glm::vec3 up = glm::normalize(glm::vec3(world[1]));

    return glm::lookAt(pos, pos - forward, up);
}

glm::mat4 CameraComponent::getProjectionMatrix() const {
	return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
}

bool CameraComponent::onAttach() {
    owner->getScene().onCameraAdded(this);
    return true;
}

void CameraComponent::onDetach() {
    if (owner->getScene().getActiveCamera() == this)
        owner->getScene().setActiveCamera(nullptr);
}

void CameraComponent::serialize(Archive& arch) const {
    arch.set("fov", fov);
    arch.set("aspect", aspect);
    arch.set("near", nearPlane);
    arch.set("far", farPlane);
}

void CameraComponent::deserialize(const Archive& arch) {
    if (!arch.get("fov", fov)) fov = 70.0f;
    if (!arch.get("aspect", aspect)) aspect = 1.25f;
    if (!arch.get("near", nearPlane)) nearPlane = 0.1f;
    if (!arch.get("far", farPlane)) farPlane = 100.0f;
}