#include "engine/input/input.h"
#include <GLFW/glfw3.h>

Input::Input(GLFWwindow* window) : m_window(window) {}

bool Input::isKeyPressed(int key) const {
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Input::isMouseButtonPressed(int button) const {
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

Vec2 Input::getMousePos() const {
    double x, y;
    glfwGetCursorPos(m_window, &x, &y);
    return Vec2((float)x, (float)y);
}

Vec2 Input::getMouseDelta() {
    double x, y;
    glfwGetCursorPos(m_window, &x, &y);

    if (m_firstMouse) {
        m_lastX = x;
        m_lastY = y;
        m_firstMouse = false;
    }

    Vec2 delta((float)(x - m_lastX), (float)(y - m_lastY));
    m_lastX = x;
    m_lastY = y;
    return delta;
}

void Input::setCursorMode(bool locked) {
    glfwSetInputMode(m_window, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}