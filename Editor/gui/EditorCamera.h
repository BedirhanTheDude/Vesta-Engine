#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class EditorCamera {
public:
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspect) const;

    void update(float dt);

    glm::vec3 position = glm::vec3(0, 2, 6);
    glm::quat rotation = glm::quat(1, 0, 0, 0);

    float fov = 70.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    float moveSpeed = 5.0f;
    float lookSpeed = 0.003f;
};