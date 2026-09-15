#include <core/Input.h>

#include <GLFW/glfw3.h>

GLFWwindow* Input::window = nullptr;
bool Input::currentKeys[350] = {};
bool Input::previousKeys[350] = {};
bool Input::currentMouse[8] = {};
bool Input::previousMouse[8] = {};
glm::vec2 Input::mousePos = {};
glm::vec2 Input::lastMousePos = {};
std::string Input::textBuffer;

void Input::init(GLFWwindow* win) {
    window = win;
    glfwSetCharCallback(window, [](GLFWwindow*, unsigned int codepoint) {
        if (codepoint < 0x80) {
            textBuffer += static_cast<char>(codepoint);
        }
        else if (codepoint < 0x800) {
            textBuffer += static_cast<char>(0xC0 | (codepoint >> 6));
            textBuffer += static_cast<char>(0x80 | (codepoint & 0x3F));
        }
        else if (codepoint < 0x10000) {
            textBuffer += static_cast<char>(0xE0 | (codepoint >> 12));
            textBuffer += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
            textBuffer += static_cast<char>(0x80 | (codepoint & 0x3F));
        }
        });
}

void Input::update() {
    clearTextInput();

	memcpy(previousKeys, currentKeys, sizeof(currentKeys));
	memcpy(previousMouse, currentMouse, sizeof(currentMouse));

    for (int i = 0; i < 350; i++)
        currentKeys[i] = glfwGetKey(window, i) == GLFW_PRESS;

    for (int i = 0; i < 8; i++)
        currentMouse[i] = glfwGetMouseButton(window, i) == GLFW_PRESS;

    lastMousePos = mousePos;
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    mousePos = glm::vec2(static_cast<float>(mx), static_cast<float>(my));
}

bool Input::isKeyDown(Key key) {
    return currentKeys[toGLFW(key)];
}

bool Input::isKeyPressed(Key key) {
    int k = toGLFW(key);
    return currentKeys[k] && !previousKeys[k];
}

bool Input::isKeyReleased(Key key) {
    int k = toGLFW(key);
    return !currentKeys[k] && previousKeys[k];
}

bool Input::isMouseDown(MouseButton button) {
    return currentMouse[toGLFW(button)];
}

bool Input::isMousePressed(MouseButton button) {
    int b = toGLFW(button);
    return currentMouse[b] && !previousMouse[b];
}

glm::vec2 Input::getMousePosition() {
    return mousePos;
}

glm::vec2 Input::getMouseDelta() {
    return mousePos - lastMousePos;
}

int Input::toGLFW(Key key) {
    return static_cast<int>(key);
}

int Input::toGLFW(MouseButton button) {
    return static_cast<int>(button);
}

std::string Input::getTextInput() {
    return textBuffer;
}

void Input::clearTextInput() {
    textBuffer.clear();
}