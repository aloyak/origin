#pragma once

#include "engine/input/keycodes.h"
#include "engine/math.h"

struct GLFWwindow;

class Input {
public:
    Input(GLFWwindow* window);

    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    
    void setCursorMode(bool locked);

    Vec2 getMousePos() const;
    Vec2 getMouseDelta();
    void resetMouseDelta();

private:
    GLFWwindow* m_window;
    double m_lastX = 0.0, m_lastY = 0.0;
    bool m_firstMouse = true;
};