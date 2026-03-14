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

#include "engine/debug/path.h"

Engine::Engine(unsigned int width, unsigned int height, const char* title) {
    Path::init();

    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%^%l%$] %v");

    m_window = new Window(width, height, title);
    m_input = new Input(m_window->getHandle());
    m_sceneManager = new SceneManager();
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
    delete m_input;
    delete m_sceneManager;

    if (m_fbo) {
        glDeleteFramebuffers(1, &m_fbo);
        glDeleteTextures(1, &m_fboTexture);
        glDeleteRenderbuffers(1, &m_rbo);
        glDeleteVertexArrays(1, &m_quadVAO);
        glDeleteBuffers(1, &m_quadVBO);
    }
}

void Engine::setupRenderTarget(unsigned int width, unsigned int height) {
    m_virtualWidth  = width;
    m_virtualHeight = height;

    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
        glDeleteTextures(1, &m_fboTexture);
        glDeleteRenderbuffers(1, &m_rbo);
        glDeleteVertexArrays(1, &m_quadVAO);
        glDeleteBuffers(1, &m_quadVBO);
        m_fbo = m_fboTexture = m_rbo = m_quadVAO = m_quadVBO = 0;
    }

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_fboTexture);
    glBindTexture(GL_TEXTURE_2D, m_fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Very important for pixelarisation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTexture, 0);

    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        spdlog::error("Framebuffer is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    if (!m_screenShader) {
        m_screenShader = std::make_unique<Shader>("assets/shaders/post_vert.glsl", "assets/shaders/post_frag.glsl");
    }
}

void Engine::resizeRenderTarget(unsigned int width, unsigned int height) {
    if (m_fbo == 0) {
        spdlog::warn("resizeRenderTarget called before setupRenderTarget — ignoring.");
        return;
    }
    if (width == m_virtualWidth && height == m_virtualHeight) return;

    m_virtualWidth  = width;
    m_virtualHeight = height;

    glBindTexture(GL_TEXTURE_2D, m_fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
}

void Engine::setPixelArt(bool enabled, int colorDepth) {
    if (enabled && m_fbo == 0) {
        spdlog::warn("setPixelArt() called before setupRenderTarget!");
    }
    m_pixelArtEnabled = enabled;
    m_colorDepth      = colorDepth;
}

void Engine::stop() {
    m_running = false;
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

    shader.setBool("u_VertexSnap", m_vertexSnap);
    shader.setFloat("u_SnapIntensity", m_snapIntensity);

    model.draw(shader);
}

bool Engine::isRunning() {
    return m_running && !SDL_QuitRequested();
}

float Engine::getTime() {
    return (float)SDL_GetTicks() / 1000.0f;
}

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

void Engine::beginFrame() {
    if (SDL_QuitRequested()) m_running = false;

    float currentFrame = (float)SDL_GetTicks() / 1000.0f;
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;

    if (m_fbo != 0) {
        // Force OpenGL to draw pixel art res
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, m_virtualWidth, m_virtualHeight);
    } else {
        Vec2 size = m_window->getSize();
        glViewport(0, 0, (int)size.x, (int)size.y);
    }

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Engine::resolveFrame() {
    if (m_fbo != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        Vec2 size = m_window->getSize();
        glViewport(0, 0, (int)size.x, (int)size.y);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_pixelArtEnabled) {
            m_screenShader->use();

            float levels = 255.0f;
            if (m_colorDepth <= 4) levels = 4.0f;
            else if (m_colorDepth <= 8) levels = 8.0f;
            else if (m_colorDepth <= 16) levels = 32.0f;
            else if (m_colorDepth >= 32) levels = 0.0f; // 0.0 disables banding in our frag shader

            m_screenShader->setFloat("colorLevels", levels);

            glBindVertexArray(m_quadVAO);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_fboTexture);
            m_screenShader->setInt("screenTexture", 0);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
    }
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