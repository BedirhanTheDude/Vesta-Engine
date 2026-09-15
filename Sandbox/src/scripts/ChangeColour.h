#pragma once

#include <scripting/Script.h>
#include <glm/common.hpp>

#include <scene/components/MaterialComponent.h>
#include <renderer/Material.h>

SCRIPT(ChangeColour)
public:
	SERIALIZE(float, speed, 1.0f);

	static constexpr float PI = glm::pi<float>();

	float timeElapsed = 0.0f;

	glm::vec3 colour = { 1.0f, 1.0f, 1.0f };
	MaterialComponent* matc;

	void onStart() override {
		matc = owner->getComponent<MaterialComponent>();
	}

	void onUpdate(float dt) override {
		if (!matc) return;

		timeElapsed += dt * speed;
		if (timeElapsed > 2 * PI) timeElapsed -= 2 * PI;

		colour.r = abs(sinf(timeElapsed + 1.4f));
		colour.g = abs(sinf(timeElapsed + 0.5f));
		colour.b = abs(sinf(timeElapsed + 2.5f));

		int i = 0;
		while (i < matc->getMaterialCount()) {
			matc->getMaterial(i)->albedo = colour;
			i++;
		}
	}

END_SCRIPT(ChangeColour);