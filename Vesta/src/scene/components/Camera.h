#pragma once

struct Camera {
	float fov = 45.0f;
	float aspect = 1.0f; // obtained from current window
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
};
