#pragma once

// What an executable plugs into Application::run
class Layer {
public:
	virtual ~Layer() = default;

	virtual void onAttach() {}
	virtual void onDetach() {}
	virtual void onUpdate(float dt) {}
};
