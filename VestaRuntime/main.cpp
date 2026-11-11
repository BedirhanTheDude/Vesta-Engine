#include <core/Application.h>

#include <EditorApp.h>

#include <memory>

int main() {
	if (!Application::init())
		return -1;

	Application::run(std::make_unique<EditorApp>());
	return 0;
}
