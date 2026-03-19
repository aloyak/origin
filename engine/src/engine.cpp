#include "engine/engine.h"
#include "engine/components/entity.h"
#include "engine/components/cameraComponent.h"

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>

#include "engine/debug/path.h"

Engine::Engine(unsigned int width, unsigned int height, const char* title) {
    Path::init();

    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%^%l%$] %v");

    m_window = std::make_unique<Window>(width, height, title);
    m_renderer = std::make_unique<Renderer>(*m_window);
    m_input = std::make_unique<Input>(m_window->getHandle());
    m_sceneManager = std::make_unique<SceneManager>();
}

Engine::~Engine() {
#ifndef __EMSCRIPTEN__
    if (ImGui::GetCurrentContext() != nullptr) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }
#endif

    m_entities.clear();
}

void Engine::stop() {
    m_running = false;
}

bool Engine::isRunning() {
    return m_running && !SDL_QuitRequested();
}

float Engine::getTime() {
    return (float)SDL_GetTicks() / 1000.0f;
}

void Engine::run(std::function<void()> mainLoop) {
#ifdef __EMSCRIPTEN__
    static std::function<void()> s_loop = [this, mainLoop]() {
        if (!m_running) { emscripten_cancel_main_loop(); return; }
        beginFrame();
        updateScene();
        resolveFrame();
        mainLoop();
        endFrame();
    };
    emscripten_set_main_loop([]() { s_loop(); }, 0, 1);
#else
    while (m_running) {
        beginFrame();
        updateScene();
        resolveFrame();
        mainLoop();
        endFrame();
    }
#endif
}

// Frame pipeline: begin, reslolve, end
void Engine::beginFrame() {
    if (SDL_QuitRequested()) m_running = false;

    float currentFrame = (float)SDL_GetTicks() / 1000.0f;
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;

    m_renderer->beginFrame();
}

void Engine::resolveFrame() {
    m_renderer->resolveFrame();
}

void Engine::endFrame() {
    m_window->swapBuffers();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
#ifndef __EMSCRIPTEN__
        if (ImGui::GetCurrentContext() != nullptr)
            ImGui_ImplSDL2_ProcessEvent(&event);
#endif
        switch (event.type) {
            case SDL_QUIT:
                m_running = false;
                break;
            case SDL_MOUSEMOTION:
#ifndef __EMSCRIPTEN__
                m_input->accumulateMouseDelta(event.motion.xrel, event.motion.yrel);
#endif
                break;
        }
    }
}

// Entity Management
Entity* Engine::createEntity(std::string name) {
    m_entities.push_back(std::make_unique<Entity>());
    m_entities.back()->name = name;
    return m_entities.back().get();
}

void Engine::destroyEntity(Entity* entity) {
    m_entities.erase(
        std::remove_if(m_entities.begin(), m_entities.end(),
            [entity](const std::unique_ptr<Entity>& e) { return e.get() == entity; }),
        m_entities.end()
    );
}

void Engine::moveToScene(Entity* entity) {
    auto it = std::find_if(m_entities.begin(), m_entities.end(),
        [entity](const std::unique_ptr<Entity>& e) { return e.get() == entity; });

    if (it != m_entities.end()) {
        if (m_sceneManager->getActiveScene()) {
            m_sceneManager->getActiveScene()->addEntity(std::move(*it));
        }
        m_entities.erase(it);
    }
}

// Scene update + render
void Engine::updateScene() {
    Camera* activeCamera = nullptr;
    const Transform* activeCameraTransform = nullptr;

    for (auto& entity : m_entities) {
        auto* camComp = entity->getComponent<CameraComponent>();
        if (camComp) {
            activeCamera           = &camComp->getCamera();
            activeCameraTransform  = &entity->transform;
            break;
        }
    }

    if (!activeCamera && m_sceneManager->getActiveScene()) {
        for (auto& entity : m_sceneManager->getActiveScene()->getEntities()) {
            auto* camComp = entity->getComponent<CameraComponent>();
            if (camComp) {
                activeCamera          = &camComp->getCamera();
                activeCameraTransform = &entity->transform;
                break;
            }
        }
    }

    if (!activeCamera) return;

    for (auto& entity : m_entities)
        entity->update(m_deltaTime);

    if (m_sceneManager->getActiveScene())
        m_sceneManager->getActiveScene()->update(m_deltaTime);

    for (auto& entity : m_entities)
        entity->render(*m_renderer, *activeCamera, *activeCameraTransform);

    if (m_sceneManager->getActiveScene())
        m_sceneManager->getActiveScene()->render(*m_renderer, *activeCamera, *activeCameraTransform);
}


// UI Management (should be moved to a separate UI layer, used for sandbox only for now)
void Engine::initUI() {
#ifndef __EMSCRIPTEN__
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplSDL2_InitForOpenGL(m_window->getHandle(), m_window->getGLContext());
    ImGui_ImplOpenGL3_Init("#version 410");
#endif
}

void Engine::beginUI() {
#ifndef __EMSCRIPTEN__
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
#endif
}

void Engine::endUI() {
#ifndef __EMSCRIPTEN__
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}