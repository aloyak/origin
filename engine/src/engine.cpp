#include "engine/engine.h"
#include "engine/components/entity.h"
#include "engine/components/cameraComponent.h"
#include "engine/components/listenerComponent.h"
#include "engine/lighting/lightingManager.h"

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>

#include "engine/utils/path.h"

Engine::Engine(unsigned int width, unsigned int height, const char* title) {
    Path::init();

    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%^%l%$] %v");

    m_window = std::make_unique<Window>(width, height, title);
    m_renderer = std::make_unique<Renderer>(*m_window);
    m_input = std::make_unique<Input>(m_window->getHandle());
    m_physicsWorld = std::make_unique<PhysicsWorld>();
    m_audioSystem = std::make_unique<AudioSystem>();
    m_sceneManager = std::make_unique<SceneManager>();

    m_lastSceneAmbient = m_renderer->getMinimumAmbientLight();
    m_renderer->setAmbientLightChangedCallback([this](float value) {
        Scene* scene = m_sceneManager ? m_sceneManager->getActiveScene() : nullptr;
        if (scene) {
            scene->setAmbientStrength(value);
        }
        m_lastSceneAmbient = value;
    });
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
        renderScene();
        resolveFrame();
        mainLoop();
        endFrame();
    };
    emscripten_set_main_loop([]() { s_loop(); }, 0, 1);
#else
    while (m_running) {
        beginFrame();

        updateScene();
        renderScene();

        resolveFrame();
        mainLoop();
        endFrame();
    }
#endif
}

// Frame pipeline: begin, reslolve, end
void Engine::beginFrame() {
    if (SDL_QuitRequested()) m_running = false;

    m_input->update();
    m_input->resetMouseDelta();
    m_input->resetScrollDelta();

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
            case SDL_MOUSEWHEEL: {
                const float direction = event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.0f : 1.0f;
                m_input->accumulateScrollDelta((float)event.wheel.x * direction, (float)event.wheel.y * direction);
                break;
            }
        }
    }

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
}

// Entity Management
Entity* Engine::createEntity(std::string name) {
    m_entities.push_back(std::make_unique<Entity>());
    m_entities.back()->name = name;
    return m_entities.back().get();
}

void Engine::destroyEntity(Entity* entity) {
    if (m_activeCamera == entity) {
        clearActiveCamera();
    }

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
    if (m_audioSystem) {
        bool listenerFound = false;
        Scene* activeScene = m_sceneManager ? m_sceneManager->getActiveScene() : nullptr;

        auto trySetListenerFromEntity = [&](Entity* entity) {
            if (listenerFound || entity == nullptr) {
                return;
            }

            auto* listener = entity->getComponent<ListenerComponent>();
            if (listener && listener->isEnabled) {
                m_audioSystem->setListenerPosition(entity->transform.position);
                listenerFound = true;
            }
        };

        for (const auto& entity : m_entities) {
            trySetListenerFromEntity(entity.get());
        }

        if (!listenerFound && activeScene) {
            for (const auto& entity : activeScene->getEntities()) {
                trySetListenerFromEntity(entity.get());
            }
        }

        auto isTrackedEntity = [&](Entity* candidate) {
            if (!candidate) {
                return false;
            }

            for (const auto& entity : m_entities) {
                if (entity.get() == candidate) {
                    return true;
                }
            }

            if (activeScene) {
                for (const auto& entity : activeScene->getEntities()) {
                    if (entity.get() == candidate) {
                        return true;
                    }
                }
            }

            return false;
        };

        if (!listenerFound && isTrackedEntity(m_activeCamera)) {
            m_audioSystem->setListenerPosition(m_activeCamera->transform.position);
            listenerFound = true;
        }

        if (!listenerFound) {
            m_audioSystem->clearListenerPosition();
        }
    }

    m_physicsWorld->stepSimulation(m_deltaTime);

    for (auto& entity : m_entities)
        entity->update(m_deltaTime);

    if (m_sceneManager->getActiveScene())
        m_sceneManager->getActiveScene()->update(m_deltaTime);
}

void Engine::renderScene() {
    Camera* activeCamera = nullptr;
    const Transform* activeCameraTransform = nullptr;
    Scene* scene = m_sceneManager->getActiveScene();

    if (scene != m_lastActiveScene) {
        m_lastActiveScene = scene;
        m_lastSceneAmbient = scene ? scene->getAmbientStrength() : -1.0f;
        if (scene) {
            m_renderer->setMinimumAmbientLight(scene->getAmbientStrength());
        }
    }
    
    if (scene) {
        float currentAmbient = m_renderer->getMinimumAmbientLight();
        if (currentAmbient != m_lastSceneAmbient) {
            scene->setAmbientStrength(currentAmbient);
            m_lastSceneAmbient = currentAmbient;
        }
    }

    auto isTrackedEntity = [this](Entity* entity) {
        if (!entity) return false;

        for (const auto& e : m_entities) {
            if (e.get() == entity) return true;
        }

        Scene* scene = m_sceneManager->getActiveScene();
        if (scene) {
            for (const auto& e : scene->getEntities()) {
                if (e.get() == entity) return true;
            }
        }

        return false;
    };

    auto tryUseCameraEntity = [&](Entity* entity) -> bool {
        if (!entity) return false;
        auto* camComp = entity->getComponent<CameraComponent>();
        if (!camComp || !camComp->isEnabled) return false;

        activeCamera = &camComp->getCamera();
        activeCameraTransform = &entity->transform;
        return true;
    };

    // First go with preferred camera
    if (m_activeCamera) {
        if (isTrackedEntity(m_activeCamera)) {
            tryUseCameraEntity(m_activeCamera);
        } else {
            m_activeCamera = nullptr;
        }
    }

    // First camera in engine-owned entities
    if (!activeCamera) {
        for (auto& entity : m_entities) {
            if (tryUseCameraEntity(entity.get())) {
                break;
            }
        }
    }

    // First camera in active scene
    if (!activeCamera && scene) {
        for (auto& entity : scene->getEntities()) {
            if (tryUseCameraEntity(entity.get())) {
                break;
            }
        }
    }

    if (!activeCamera) return;

    // Update lighting system with all active entities
    std::vector<Entity*> allEntities;
    for (auto& entity : m_entities)
        allEntities.push_back(entity.get());
    if (scene) {
        for (auto& entity : scene->getEntities())
            allEntities.push_back(entity.get());
    }
    LightingManager::instance().updateLights(allEntities);

    for (auto& entity : m_entities)
        entity->render(*m_renderer, *activeCamera, *activeCameraTransform);

    if (scene)
        scene->render(*m_renderer, *activeCamera, *activeCameraTransform);
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