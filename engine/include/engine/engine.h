#pragma once

#include "engine/transform.h"

struct GLFWwindow;
class Shader;
class Mesh;
class Camera;

class Engine {
public:
    Engine(unsigned int width = 640, unsigned int height = 480, const char* title = "Origin Engine");    
    ~Engine();

    bool isRunning();
    void beginFrame();
    void render(const Mesh& mesh, Shader& shader, const Camera& camera, const Transform& transform);
    void endFrame();
    
    float getTime();
private:
    GLFWwindow* m_window;
};