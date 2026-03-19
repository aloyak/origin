#pragma once

#include "engine/core/transform.h"
#include "engine/core/window.h"
#include "engine/input/input.h"
#include "engine/render/render.h"
#include "engine/scene/sceneManager.h"

#include <functional>
#include <vector>
#include <memory>
#include <string>

#ifndef __EMSCRIPTEN__
    #include <imgui.h>
    #include <imgui_impl_sdl2.h>
    #include <imgui_impl_opengl3.h>
#endif

class Entity;

class Engine {
public:
    Engine(unsigned int width = 640, unsigned int height = 480, const char* title = "Origin Engine");
    ~Engine();

    void run(std::function<void()> mainLoop);
    void stop();

    bool isRunning();

    float getTime();
    float getDeltaTime() const { return m_deltaTime; }

    // Core
    Input& getInput() { return *m_input; }
    SceneManager& getSceneManager() { return *m_sceneManager; }
    Window& getWindow() { return *m_window; }
    Renderer& getRenderer() { return *m_renderer; }

    // Entity management
    Entity* createEntity(std::string name = "Entity");
    void    destroyEntity(Entity* entity);
    void    moveToScene(Entity* entity);

    // Update & render all entities and the active scene
    // Entities are expected to call renderer.render() themselves
    void updateScene();
    void renderScene();

    // UI
    void initUI();
    void beginUI();
    void endUI();
#ifndef __EMSCRIPTEN__
    ImGuiIO& getIO() { return ImGui::GetIO(); }
#endif

private:
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Input> m_input;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<SceneManager> m_sceneManager;

    void beginFrame();
    void resolveFrame();
    void endFrame();

    float m_deltaTime = 0.0f;
    float m_lastFrame = 0.0f;

    std::vector<std::unique_ptr<Entity>> m_entities;

    bool m_running = true;
};