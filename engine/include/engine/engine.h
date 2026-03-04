#pragma once

#include "engine/transform.h"
#include "engine/input/input.h"

#include <vector>
#include <memory>

struct GLFWwindow;
class Shader;
class Camera;
class Model;
class Entity;

class Engine {
public:
    Engine(unsigned int width = 640, unsigned int height = 480, const char* title = "Origin Engine");    
    ~Engine();

    void setVerbose(int verbose);

    bool isRunning();
    void beginFrame();
    void endFrame();

    Entity* createEntity();
    void destroyEntity(Entity* entity);

    void updateScene();

    // prefer updateScene() over calling this directly
    void render(Model& model, Shader& shader, const Camera& camera, const Transform& cameraTransform, const Transform& modelTransform);

    float getTime();
    float getDeltaTime() const { return m_deltaTime; }
    Input& getInput() { return *m_input; }

private:
    GLFWwindow* m_window;
    Input*      m_input;

    float m_deltaTime = 0.0f;
    float m_lastFrame = 0.0f;

    std::vector<std::unique_ptr<Entity>> m_entities;
};