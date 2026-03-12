#pragma once

#include "engine/transform.h"
#include "engine/input/input.h"
#include "engine/scene/sceneManager.h"

#include <functional>
#include <vector>
#include <memory>
#include <string>

struct SDL_Window;

class Shader;
class Camera;
class Model;
class Entity;

class Engine {
public:
    Engine(unsigned int width = 640, unsigned int height = 480, const char* title = "Origin Engine");    
    ~Engine();

    void run(std::function<void()> mainLoop);
    void stop();

    bool isRunning();

    void setPixelArtSettings(unsigned int virtualWidth, unsigned int virtualHeight, int colorDepth = 32);
    void setVertexSnap(bool enabled, float intensity = 40.0f) { 
        m_vertexSnap = enabled; 
        m_snapIntensity = intensity; 
    }

    Entity* createEntity(std::string name = "Entity");
    void destroyEntity(Entity* entity);
    void moveToScene(Entity* entity);

    void updateScene();

    // prefer updateScene() over calling this directly
    void render(Model& model, Shader& shader, const Camera& camera, const Transform& cameraTransform, const Transform& modelTransform);

    float getTime();
    float getDeltaTime() const { return m_deltaTime; }

    Input& getInput() { return *m_input; }
    SceneManager& getSceneManager() { return *m_sceneManager; }

    // window
    void setFullscreen(bool fullscreen);
    void setWindowTitle(const char* title);
    void enableVSync(bool enabled);
    float getAspectRatio() const;

private:
    SDL_Window* m_window;
    Input*      m_input;
    SceneManager* m_sceneManager;

    void* m_glContext; // SDL_GLContext type, just not included

    void beginFrame();
    void endFrame();

    float m_deltaTime = 0.0f;
    float m_lastFrame = 0.0f;

    std::vector<std::unique_ptr<Entity>> m_entities;

    bool m_running = true;

    // Pixel art framebuffer
    unsigned int m_fbo = 0;
    unsigned int m_fboTexture = 0;
    unsigned int m_rbo = 0;
    unsigned int m_quadVAO = 0;
    unsigned int m_quadVBO = 0;
    
    unsigned int m_virtualWidth = 0;
    unsigned int m_virtualHeight = 0;
    int m_colorDepth = 32;
    std::unique_ptr<Shader> m_screenShader;

    bool m_vertexSnap = false;
    float m_snapIntensity = 40.0f;
};