#include <glad/glad.h>
#include <GLFW/glfw3.h>

// stb implementation lives in Engine (stb_impl.cpp); just use the declarations here.
#include <stb_image.h>

#include <iostream>
#include <filesystem>

#include <imgui.h>
#include <ImGuizmo.h>

#include <core/Application.h>
#include <core/Input.h>
#include <core/Project.h>

#include <scene/Scene.h>
#include <scene/Entity.h>
#include <scene/components/CameraComponent.h>
#include <scene/components/TransformComponent.h>

#include <controller/EntityController.h>
#include <controller/SceneController.h>

#include <persistance/SceneSerializer.h>
#include <persistance/Archive.h>

#include <renderer/Renderer.h>
#include <renderer/ViewportFramebuffer.h>

#include <scripting/ScriptLoader.h>
#include <scripting/ScriptCompiler.h>

#include <gui/ImGuiLayer.h>
#include <gui/EditorLayer.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

// command line argument (a directory is made into a project if it isn't one yet), else the last opened project,
// else the sample project in the repo
static bool openProject(int argc, char** argv) {
	static const char* editorConfigName = "editor.dat";

	bool opened = false;

	if (argc > 1) {
		// argv is in the ANSI code page, so this is the narrow (not u8path) constructor
		std::filesystem::path requested(argv[1]);
		opened = Project::create(requested, requested.filename().string());
	}
	else {
		Archive editorConfig;
		std::string lastProject;
		if (editorConfig.loadFromFile(editorConfigName) && editorConfig.get("last_project", lastProject))
			opened = Project::open(std::filesystem::u8path(lastProject));
	}

	if (!opened) return false;

	Archive editorConfig;
	editorConfig.set("last_project", Project::getRoot().string());
	editorConfig.saveToFile(editorConfigName);

	return true;
}

int main(int argc, char** argv) {
	if (!openProject(argc, argv)) {
		std::cout << "Failed to open a project\n";
	}

	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW\n";
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(1000, 800, "Vesta Engine", nullptr, nullptr);
	if (!window) {
		std::cout << "Failed to create window.\n";
		glfwTerminate();
		return -1;
	}

	Application::setWindow(window); // store a global pointer to the window

	int iconW, iconH, iconChannels;
	unsigned char* iconPixels = stbi_load("icon.png", &iconW, &iconH, &iconChannels, 4);
	if (iconPixels) {
		GLFWimage icon = { iconW, iconH, iconPixels };
		glfwSetWindowIcon(window, 1, &icon);
		stbi_image_free(iconPixels);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, 1000, 800);


	// TODO: Write an actual polling system for script modification, until then leave poll() commented out

	// compile() emits temp_scripts.dll; replaceOldDLLFile() renames it to scripts.dll
	// (the reload path does this too, but the initial compile needs it explicitly).
	// Both live in <project>/.vesta/build
	if (!ScriptLoader::scriptsDLLExists()) {
		if (ScriptCompiler::compile())
			ScriptLoader::replaceOldDLLFile();
	}
	ScriptCompiler::loadDLL();

	Input::init(window);

	// Application owns the Scene and Renderer; controllers reach them through Application,
	// so the editor panels and this loop share one source of truth.
	// newScene loads the project's startup scene if there is one, otherwise builds the default scene.
	Scene* scene = Application::newScene(Project::getStartupScene());
	Renderer* renderer = Application::newRenderer();

	ImGuiLayer imguiLayer;
	imguiLayer.Init(window);

	EditorLayer editorLayer;

	auto onExit = []() {
		glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
		};

	editorLayer.Init(onExit);
	editorLayer.initFileManager(Project::getRoot());

	ViewportFramebuffer gameFramebuffer;
	gameFramebuffer.Init(1000, 800);

	ViewportFramebuffer sceneFramebuffer;
	sceneFramebuffer.Init(1000, 800);

	float last = 0.0f;

	while (!glfwWindowShouldClose(window)) {
		float now = static_cast<float>(glfwGetTime());
		if (last == 0.0f) {
			last = now;
		}

		float dt = now - last;
		last = now;

		glfwPollEvents();

		Input::update();

		if (Input::isKeyDown(Key::LeftControl) && Input::isKeyPressed(Key::S) && !scene->isPlaying) {
			SceneController::saveScene();
			printf("Scene saved!\n");
		}

		if (Input::isKeyDown(Key::LeftControl) && Input::isKeyPressed(Key::O)) {
			SceneController::loadScene();
			editorLayer.OnEntityRemoved(nullptr);
			printf("Scene loaded!\n");
		}

		if (Input::isKeyDown(Key::LeftControl) &&
			Input::isKeyDown(Key::LeftShift) &&
			Input::isKeyPressed(Key::F)) {

			Entity camEntity = scene->getActiveCameraEntity();
			if (camEntity.isValid()) {
				TransformComponent camTransform = camEntity.getTransform();
				camTransform.setPosition(editorLayer.editorCamera.position);
				camTransform.setRotation(editorLayer.editorCamera.rotation);
			}
		}

		bool isDragging = editorLayer.inspectorFocused &&
			ImGui::IsAnyItemActive() &&
			ImGui::IsMouseDragging(ImGuiMouseButton_Left);

		if (isDragging) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		//ScriptCompiler::poll(dt, *scene);

		editorLayer.editorCamera.update(dt);
		scene->onUpdate(dt);

		// Scene view — editor camera
		ImVec2 sceneSize = editorLayer.getSceneViewSize();
		unsigned int sw = static_cast<unsigned int>(sceneSize.x);
		unsigned int sh = static_cast<unsigned int>(sceneSize.y);

		if (sw > 0 && sh > 0) {
			sceneFramebuffer.Resize(sw, sh);
			float aspect = static_cast<float>(sw) / static_cast<float>(sh);
			sceneFramebuffer.Bind();
			renderer->render(*scene,
				editorLayer.editorCamera.getViewMatrix(),
				editorLayer.editorCamera.getProjectionMatrix(aspect));
			sceneFramebuffer.Unbind();
			editorLayer.sceneViewTexture = sceneFramebuffer.GetColorAttachment();
		}

		// Game view — scene camera
		ImVec2 gameSize = editorLayer.getGameViewSize();
		unsigned int gw = static_cast<unsigned int>(gameSize.x);
		unsigned int gh = static_cast<unsigned int>(gameSize.y);

		auto gameCamera = scene->getActiveCameraEntity().getComponent<CameraComponent>(); // empty if there is no active camera

		if (gw > 0 && gh > 0 && gameCamera) {
			gameFramebuffer.Resize(gw, gh);
			gameCamera->setAspect(static_cast<float>(gw) / static_cast<float>(gh));
			gameFramebuffer.Bind();
			renderer->render(*scene);
			gameFramebuffer.Unbind();
			editorLayer.gameViewTexture = gameFramebuffer.GetColorAttachment();
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		int windowWidth = 0;
		int windowHeight = 0;
		glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
		glViewport(0, 0, windowWidth, windowHeight);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		imguiLayer.Begin();
		ImGuizmo::BeginFrame();
		editorLayer.OnUIRender();
		imguiLayer.End();

		glfwSwapBuffers(window);
	}

	Application::shutdown();
	imguiLayer.Shutdown();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
