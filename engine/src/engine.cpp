#include "engine/engine.h"
#include "engine/mesh.h"
#include "engine/shader.h"
#include "engine/model.h"
#include "engine/camera.h"
#include "engine/components/entity.h"
#include "engine/components/camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>
    #include <emscripten.h>
#else
    #include <glad/glad.h>
#endif

#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>
#include <algorithm>

int frameBufferSizeCallback(void* userdata, SDL_Event* event) {
    if (event->type == SDL_WINDOWEVENT) {
        if (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            int width = event->window.data1;
            int height = event->window.data2;
            glViewport(0, 0, width, height);
        }
    }
    return 0;
}

Engine::Engine(unsigned int width, unsigned int height, const char* title) {
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%^%l%$] %v");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        spdlog::error("Failed to initialize SDL: {}", SDL_GetError());
        SDL_Quit();
    }

#ifdef __EMSCRIPTEN__ 
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else // OpenGL 4.1 as it is the last supported by MacOS
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

    m_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
    if (!m_window) {
        spdlog::error("Failed to create SDL window: {}", SDL_GetError());
        SDL_Quit();
        throw std::runtime_error("SDL window creation failed");
    }
    
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        spdlog::error("Failed to create GL context: {}", SDL_GetError());
        SDL_Quit();
    }
    SDL_GL_MakeCurrent(m_window, m_glContext);
    
    m_input = new Input(m_window);
    m_sceneManager = new SceneManager();
    
#ifndef __EMSCRIPTEN__ 
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        spdlog::error("Failed to init GLAD!");
    }
#endif
    spdlog::info("Using OpenGL {}", (const char*)glGetString(GL_VERSION));
    
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);
    
    SDL_SetWindowResizable(m_window, SDL_TRUE);
    SDL_AddEventWatch(frameBufferSizeCallback, m_window);
}

void Engine::stop() {
    m_running = false;
}

void Engine::run(std::function<void()> mainLoop) {
#ifdef __EMSCRIPTEN__
    static std::function<void()> s_loop = [this, mainLoop]() {
        if (!m_running) { emscripten_cancel_main_loop(); return; }
        beginFrame();
        mainLoop();
        updateScene();
        endFrame();
    };
    emscripten_set_main_loop([]() { s_loop(); }, 0, 1);
#else
    while (m_running) {
        beginFrame();
        mainLoop();
        updateScene();
        endFrame();
    }
#endif
}

void Engine::render(Model& model, Shader& shader, const Camera& camera, const Transform& cameraTransform, const Transform& modelTransform) {
    shader.use();

    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(modelTransform.position.x, modelTransform.position.y, modelTransform.position.z));
    modelMat = glm::rotate(modelMat, glm::radians(modelTransform.rotation.z), glm::vec3(0, 0, 1));
    modelMat = glm::rotate(modelMat, glm::radians(modelTransform.rotation.y), glm::vec3(0, 1, 0));
    modelMat = glm::rotate(modelMat, glm::radians(modelTransform.rotation.x), glm::vec3(1, 0, 0));
    modelMat = glm::scale(modelMat, glm::vec3(modelTransform.scale.x, modelTransform.scale.y, modelTransform.scale.z));

    shader.setMat4("u_Model", &modelMat);
    shader.setMat4("u_View", camera.getViewMatrix(cameraTransform));
    shader.setMat4("u_Projection", camera.getProjectionMatrix());

    model.draw(shader);
}

bool Engine::isRunning() {
    return m_running && !SDL_QuitRequested();
}

float Engine::getTime() {
    return (float)SDL_GetTicks() / 1000.0f;
}

void Engine::beginFrame() {
    if (SDL_QuitRequested()) m_running = false;

    float currentFrame = (float)SDL_GetTicks() / 1000.0f;
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Engine::endFrame() {
    SDL_GL_SwapWindow(m_window);
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
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


Engine::~Engine() {
    m_entities.clear();
    delete m_input;
    delete m_sceneManager;

    SDL_GL_DeleteContext(m_glContext);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

// Window Management

void Engine::setFullscreen(bool fullscreen) {
    if (fullscreen)
        SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    else
        SDL_SetWindowFullscreen(m_window, 0);
}

void Engine::enableVSync(bool enabled) {
    if (enabled)
        SDL_GL_SetSwapInterval(1);
    else
        SDL_GL_SetSwapInterval(0);
}

void Engine::setWindowTitle(const char* title) {
    SDL_SetWindowTitle(m_window, title);
}

float Engine::getAspectRatio() const {
    int w, h;
    SDL_GetWindowSize(m_window, &w, &h);
    return (float)w / (float)h;
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

void Engine::updateScene() {
    Camera* activeCamera = nullptr;
    const Transform* activeCameraTransform = nullptr;
    for (auto& entity : m_entities) {
        auto* camComp = entity->getComponent<CameraComponent>();
        if (camComp) {
            activeCamera = &camComp->getCamera();
            activeCameraTransform = &entity->transform;
            break;
        }
    }

    if (!activeCamera && m_sceneManager->getActiveScene()) {
        for (auto& entity : m_sceneManager->getActiveScene()->getEntities()) {
            auto* camComp = entity->getComponent<CameraComponent>();
            if (camComp) {
                activeCamera = &camComp->getCamera();
                activeCameraTransform = &entity->transform;
                break;
            }
        }
    }

    if (!activeCamera) {
        spdlog::warn("No active camera found in engine or active scene!");
        return;
    }

    for (auto& entity : m_entities) {
        entity->update(m_deltaTime);
    }

    if (m_sceneManager->getActiveScene()) {
        m_sceneManager->getActiveScene()->update(m_deltaTime);
    }

    for (auto& entity : m_entities) {
        entity->render(*this, *activeCamera, *activeCameraTransform);
    }

    if (m_sceneManager->getActiveScene()) {
        m_sceneManager->getActiveScene()->render(*this, *activeCamera, *activeCameraTransform);
    }
}