#include "engine.h"

#include <glad/glad.h> // Here so not leaked to game
#include <GLFW/glfw3.h> 
#include <spdlog/spdlog.h> 

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

Engine::Engine(unsigned int width, unsigned int height, const char* title) : m_window(nullptr) {
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%^%l%$] %v");

    if (!glfwInit()) {
        spdlog::error("Failed to initialize GLFW");
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

    glViewport(0, 0, 640, 480);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
}

bool Engine::isRunning() {
    return !glfwWindowShouldClose(m_window);
}

void Engine::beginFrame() {
    //
}

void Engine::endFrame() {
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

Engine::~Engine() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}