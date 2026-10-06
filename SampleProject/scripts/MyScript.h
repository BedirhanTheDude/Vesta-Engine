#pragma once

#include <scripting/Script.h>

#include <scene/Entity.h>
#include <scene/components/TransformComponent.h>

#include <glm/glm.hpp>

BEGIN_SCRIPT(MyScript)
public:
	// Runs once when the script is first attached or at scene start
	void onStart() override {

	}

	// Runs once every frame
	void onUpdate(float dt) override {
		TransformComponent transform = entity.getTransform();

		transform.rotateAroundLocalAxis(glm::vec3(0, 1, 0), 90.0f * dt );
	}
END_SCRIPT(MyScript)
