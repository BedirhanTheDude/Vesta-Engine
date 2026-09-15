#include <core/Application.h>

#include <scene/Scene.h>
#include <renderer/Renderer.h>

#include <persistance/SceneSerializer.h>

// definitions for the private static members declared in Application.h
std::unique_ptr<Scene> Application::scene;
std::unique_ptr<Renderer> Application::renderer;

Scene* Application::newScene(const std::string& name) {
	// Scene's ctor is private with Application as friend; make_unique isn't the friend,
	// so construct directly and adopt into the unique_ptr.
	scene = std::unique_ptr<Scene>(new Scene(name));

	bool loaded = SceneSerializer::load(*scene, false);
	if (!loaded)
		Scene::createDefaultScene(*scene);

	return scene.get();
}

Scene* Application::getCurrentScene() {
	return scene.get();
}

Renderer* Application::newRenderer() {
	renderer = std::unique_ptr<Renderer>(new Renderer());
	return renderer.get();
}

Renderer* Application::getCurrentRenderer() {
	return renderer.get();
}

void Application::shutdown() {
	scene.reset();
	renderer.reset();
}