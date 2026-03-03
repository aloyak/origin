#include "engine/engine.h"
#include "engine/mesh.h"
#include "engine/shader.h"
#include "engine/camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h> // Here so not leaked to game
#include <GLFW/glfw3.h> 
#include <spdlog/spdlog.h> 

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
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

    glfwMakeContextCurrent(m_window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        spdlog::error("Failed to init GLAD!");
    }
    spdlog::info("Using OpenGL {}", (const char*)glGetString(GL_VERSION));
    
    glViewport(0, 0, width, height); 
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
}

bool Engine::isRunning() {
    return !glfwWindowShouldClose(m_window);
}

void Engine::render(const Mesh& mesh, Shader& shader, const Camera& camera, const Transform& transform) {
    shader.use();

    glm::mat4 model = glm::mat4(1.0f);
    
    model = glm::translate(model, glm::vec3(transform.position.x, transform.position.y, transform.position.z));
    
    model = glm::rotate(model, glm::radians(transform.rotation.z), glm::vec3(0, 0, 1));
    model = glm::rotate(model, glm::radians(transform.rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(transform.rotation.x), glm::vec3(1, 0, 0));
    
    model = glm::scale(model, glm::vec3(transform.scale.x, transform.scale.y, transform.scale.z));

    shader.setMat4("u_Model", &model);
    shader.setMat4("u_View", camera.getViewMatrix());
    shader.setMat4("u_Projection", camera.getProjectionMatrix());

    mesh.draw();
}

float Engine::getTime() {
    return (float)glfwGetTime();
}

void Engine::beginFrame() {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Engine::endFrame() {
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

Engine::~Engine() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}