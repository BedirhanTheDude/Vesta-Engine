#pragma once

#include <glm/fwd.hpp>

#include <scene/components/Component.h>

class Archive;

class CameraComponent : public Component {
public:
	CameraComponent(float fov = 45.0f, float aspect = 1.0f, float nearPlane = 0.1f, float farPlane = 100.0f) 
		: fov(fov), aspect(aspect), nearPlane(nearPlane), farPlane(farPlane) {
		registerProperty("FOV", PropertyType::Float, &this->fov);
		registerProperty("Near", PropertyType::Float, &this->nearPlane);
		registerProperty("Far", PropertyType::Float, &this->farPlane);
	}

	float fov;
	float aspect; // obtained from current window
	float nearPlane;
	float farPlane;

	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;

	bool onAttach();
	void onDetach();

	void serialize(Archive& arch) const override;
	void deserialize(const Archive& arch) override;
};