#include "engine/input/input.h"
#include <GLFW/glfw3.h>

Input::Input(GLFWwindow* window) : m_window(window) {}

bool Input::isKeyPressed(int key) const {
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Input::isMouseButtonPressed(int button) const {
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

void Input::getMousePos(double& x, double& y) const {
    glfwGetCursorPos(m_window, &x, &y);
}

void Input::setCursorMode(bool locked) {
    glfwSetInputMode(m_window, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}