#include <gui/EditorCamera.h>

#include <core/Input.h>
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 EditorCamera::getViewMatrix() const {
    glm::mat4 rot = glm::mat4_cast(rotation);
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
    return glm::inverse(trans * rot);
}

glm::mat4 EditorCamera::getProjectionMatrix(float aspect) const {
    return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
}

void EditorCamera::update(float dt) {
    if (!Input::isMouseDown(MouseButton::Right)) return;

    glm::vec2 delta = Input::getMouseDelta();
    float yaw = -delta.x * lookSpeed;
    float pitch = -delta.y * lookSpeed;

    glm::quat yawRot = glm::angleAxis(yaw, glm::vec3(0, 1, 0));
    glm::quat pitchRot = glm::angleAxis(pitch, rotation * glm::vec3(1, 0, 0));
    rotation = yawRot * pitchRot * rotation;

    glm::vec3 forward = rotation * glm::vec3(0, 0, -1);
    glm::vec3 right = rotation * glm::vec3(1, 0, 0);
    glm::vec3 up = glm::vec3(0, 1, 0);

    float speed = moveSpeed * dt;
    if (Input::isKeyDown(Key::LeftShift)) speed *= 3.0f;

    if (Input::isKeyDown(Key::W)) position += forward * speed;
    if (Input::isKeyDown(Key::S)) position -= forward * speed;
    if (Input::isKeyDown(Key::D)) position += right * speed;
    if (Input::isKeyDown(Key::A)) position -= right * speed;
    if (Input::isKeyDown(Key::E)) position += up * speed;
    if (Input::isKeyDown(Key::Q)) position -= up * speed;
}