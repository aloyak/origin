#include "engine/engine.h"
#include "engine/mesh.h"
#include "engine/shader.h"
#include "engine/model.h"
#include "engine/camera.h"
#include "engine/components/entity.h"
#include "engine/components/camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <algorithm>

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

Engine::Engine(unsigned int width, unsigned int height, const char* title) {
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%^%l%$] %v");

    if (!glfwInit()) {
        spdlog::error("Failed to initialize GLFW!");
        glfwTerminate();
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!m_window) {
        spdlog::error("Failed to create GLFW window");
        glfwTerminate();
    }

    m_input = new Input(m_window);

    glfwMakeContextCurrent(m_window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        spdlog::error("Failed to init GLAD!");
    }
    spdlog::info("Using OpenGL {}", (const char*)glGetString(GL_VERSION));
    
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height); 
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
}

Entity* Engine::createEntity() {
    m_entities.push_back(std::make_unique<Entity>());
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

    if (!activeCamera) {
        spdlog::warn("No active camera found in scene!");
        return;
    }

    for (auto& entity : m_entities)
        entity->update(m_deltaTime);

    for (auto& entity : m_entities)
        entity->render(*this, *activeCamera, *activeCameraTransform);
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

void Engine::setVerbose(int verbose) {
    switch (verbose) {
        case 0: spdlog::set_level(spdlog::level::off);   break;
        case 1: spdlog::set_level(spdlog::level::err);   break;
        case 2: spdlog::set_level(spdlog::level::warn);  break;
        case 3: spdlog::set_level(spdlog::level::info);  break;
        case 4: spdlog::set_level(spdlog::level::debug); break;
        case 5: spdlog::set_level(spdlog::level::trace); break;
        default: spdlog::set_level(spdlog::level::info); break;
    }
}

bool Engine::isRunning() {
    return !glfwWindowShouldClose(m_window);
}

float Engine::getTime() {
    return (float)glfwGetTime();
}

void Engine::beginFrame() {
    float currentFrame = (float)glfwGetTime();
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Engine::endFrame() {
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}


Engine::~Engine() {
    m_entities.clear();
    delete m_input;
    glfwDestroyWindow(m_window);
    glfwTerminate();
}