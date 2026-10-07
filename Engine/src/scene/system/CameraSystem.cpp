#include "CameraSystem.h"

#include <scene/system/TransformSystem.h>

#include <persistance/Archive.h>

#include <glm/gtc/matrix_transform.hpp>

namespace CameraSystem {

    glm::mat4 getViewMatrix(const Transform& transform, Scene* scene) {
        glm::mat4 world = TransformSystem::getMatrix(transform, scene);

        glm::vec3 pos = glm::vec3(world[3]);
        glm::vec3 forward = glm::normalize(glm::vec3(world[2]));
        glm::vec3 up = glm::normalize(glm::vec3(world[1]));

        return glm::lookAt(pos, pos - forward, up);
    }

    glm::mat4 getProjectionMatrix(const Camera& camera) {
        return glm::perspective(glm::radians(camera.fov), camera.aspect, camera.nearPlane, camera.farPlane);
    }

    void serialize(const Camera& camera, Archive& arch) {
        arch.set("fov", camera.fov);
        arch.set("aspect", camera.aspect);
        arch.set("near", camera.nearPlane);
        arch.set("far", camera.farPlane);
    }

    void deserialize(Camera& camera, const Archive& arch) {
        if (!arch.get("fov", camera.fov)) camera.fov = 70.0f;
        if (!arch.get("aspect", camera.aspect)) camera.aspect = 1.25f;
        if (!arch.get("near", camera.nearPlane)) camera.nearPlane = 0.1f;
        if (!arch.get("far", camera.farPlane)) camera.farPlane = 100.0f;
    }
}
