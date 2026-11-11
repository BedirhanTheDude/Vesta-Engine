#include <core/Input.h>

#include <GLFW/glfw3.h>

GLFWwindow* Input::m_Window = nullptr;
bool Input::m_CurrentKeys[350] = {};
bool Input::m_PreviousKeys[350] = {};
bool Input::m_CurrentMouse[8] = {};
bool Input::m_PreviousMouse[8] = {};
glm::vec2 Input::m_MousePos = {};
glm::vec2 Input::m_LastMousePos = {};
std::string Input::m_TextBuffer;

void Input::init(GLFWwindow* win) {
    m_Window = win;
    glfwSetCharCallback(m_Window, [](GLFWwindow*, unsigned int codepoint) {
        if (codepoint < 0x80) {
            m_TextBuffer += static_cast<char>(codepoint);
        }
        else if (codepoint < 0x800) {
            m_TextBuffer += static_cast<char>(0xC0 | (codepoint >> 6));
            m_TextBuffer += static_cast<char>(0x80 | (codepoint & 0x3F));
        }
        else if (codepoint < 0x10000) {
            m_TextBuffer += static_cast<char>(0xE0 | (codepoint >> 12));
            m_TextBuffer += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
            m_TextBuffer += static_cast<char>(0x80 | (codepoint & 0x3F));
        }
        });
}

void Input::update() {
    clearTextInput();

	memcpy(m_PreviousKeys, m_CurrentKeys, sizeof(m_CurrentKeys));
	memcpy(m_PreviousMouse, m_CurrentMouse, sizeof(m_CurrentMouse));

    for (int i = 0; i < 350; i++)
        m_CurrentKeys[i] = glfwGetKey(m_Window, i) == GLFW_PRESS;

    for (int i = 0; i < 8; i++)
        m_CurrentMouse[i] = glfwGetMouseButton(m_Window, i) == GLFW_PRESS;

    m_LastMousePos = m_MousePos;
    double mx, my;
    glfwGetCursorPos(m_Window, &mx, &my);
    m_MousePos = glm::vec2(static_cast<float>(mx), static_cast<float>(my));
}

bool Input::isKeyDown(Key key) {
    return m_CurrentKeys[toGLFW(key)];
}

bool Input::isKeyPressed(Key key) {
    int k = toGLFW(key);
    return m_CurrentKeys[k] && !m_PreviousKeys[k];
}

bool Input::isKeyReleased(Key key) {
    int k = toGLFW(key);
    return !m_CurrentKeys[k] && m_PreviousKeys[k];
}

bool Input::isMouseDown(MouseButton button) {
    return m_CurrentMouse[toGLFW(button)];
}

bool Input::isMousePressed(MouseButton button) {
    int b = toGLFW(button);
    return m_CurrentMouse[b] && !m_PreviousMouse[b];
}

glm::vec2 Input::getMousePosition() {
    return m_MousePos;
}

glm::vec2 Input::getMouseDelta() {
    return m_MousePos - m_LastMousePos;
}

int Input::toGLFW(Key key) {
    return static_cast<int>(key);
}

int Input::toGLFW(MouseButton button) {
    return static_cast<int>(button);
}

std::string Input::getTextInput() {
    return m_TextBuffer;
}

void Input::clearTextInput() {
    m_TextBuffer.clear();
}