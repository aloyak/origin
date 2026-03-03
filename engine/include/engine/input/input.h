#pragma once

#include "engine/math.h"

struct GLFWwindow;

class Input {
public:
    Input(GLFWwindow* window);

    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    
    void getMousePos(double& x, double& y) const;
    void setCursorMode(bool locked);

private:
    GLFWwindow* m_window;
};