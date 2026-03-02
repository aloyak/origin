#pragma once

struct GLFWwindow;

class Engine {
public:
    Engine(unsigned int width = 640, unsigned int height = 480, const char* title = "Origin Engine");    
    ~Engine();

    bool isRunning();
    void beginFrame();
    void endFrame();

private:
    GLFWwindow* m_window;
};