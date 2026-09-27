#pragma once

#include <glm/glm.hpp>

#include <scene/components/Component.h>

class Archive;

class CameraComponent : public Component {
public:
	explicit CameraComponent(const Entity& entity) : Component(entity) {}

	// what addComponent<CameraComponent>(fov, aspect, near, far) forwards to
	void init(float fov = 45.0f, float aspect = 1.0f, float nearPlane = 0.1f, float farPlane = 100.0f);

	float getFov() const;
	void setFov(float fov);

	float getAspect() const;
	void setAspect(float aspect);

	float getNearPlane() const;
	void setNearPlane(float nearPlane);

	float getFarPlane() const;
	void setFarPlane(float farPlane);

	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;

	bool onAttach();
	void onDetach();

	void serialize(Archive& arch) const;
	void deserialize(const Archive& arch);
};
