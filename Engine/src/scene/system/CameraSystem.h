#pragma once

#include <glm/glm.hpp>

#include <scene/components/Camera.h>
#include <scene/components/Transform.h>

class Archive;
class Scene;

namespace CameraSystem {

	glm::mat4 getViewMatrix(const Transform& transform, Scene* scene = nullptr);
	glm::mat4 getProjectionMatrix(const Camera& camera);

	void serialize(const Camera& camera, Archive& arch);
	void deserialize(Camera& camera, const Archive& arch);
}
