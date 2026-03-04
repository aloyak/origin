#pragma once

#include "engine/transform.h"
#include "engine/input/input.h"

struct GLFWwindow;
class Shader;
class Mesh;
class Camera;
class Model;
class Texture;

class Engine {
public:
    Engine(unsigned int width = 640, unsigned int height = 480, const char* title = "Origin Engine");    
    ~Engine();

    void setVerbose(int verbose);

    bool isRunning();
    void beginFrame();
    void render(Model& model, Shader& shader, const Camera& camera, const Transform& transform);
    
    void endFrame();
    
    float getTime();
    float getDeltaTime() const { return m_deltaTime; }

    Input& getInput() { return *m_input; }

private:
    GLFWwindow* m_window;
    Input* m_input;

    float m_deltaTime, m_lastFrame;
};