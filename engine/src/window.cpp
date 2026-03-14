#include "engine/window.h"

#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>

static int frameBufferSizeCallback(void* userdata, SDL_Event* event) {
    if (event->type == SDL_WINDOWEVENT &&
        event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        glViewport(0, 0, event->window.data1, event->window.data2);
    }
    return 0;
}

Window::Window(unsigned int width, unsigned int height, const char* title) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        spdlog::error("Failed to initialize SDL: {}", SDL_GetError());
        SDL_Quit();
        throw std::runtime_error("SDL initialization failed");
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

    m_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!m_window) {
        spdlog::error("Failed to create SDL window: {}", SDL_GetError());
        SDL_Quit();
        throw std::runtime_error("SDL window creation failed");
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        spdlog::error("Failed to create GL context: {}", SDL_GetError());
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        throw std::runtime_error("GL context creation failed");
    }
    SDL_GL_MakeCurrent(m_window, m_glContext);

#ifndef __EMSCRIPTEN__
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        spdlog::error("Failed to init GLAD!");
#endif

    spdlog::info("Using OpenGL {}", (const char*)glGetString(GL_VERSION));

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);

    SDL_AddEventWatch(frameBufferSizeCallback, m_window);
}

Window::~Window() {
    SDL_GL_DeleteContext(m_glContext);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void Window::swapBuffers() {
    SDL_GL_SwapWindow(m_window);
}

void Window::enableVSync(bool enabled) {
    SDL_GL_SetSwapInterval(enabled ? 1 : 0);
}

void Window::setWindowTitle(const char* title) {
    SDL_SetWindowTitle(m_window, title);
}

float Window::getAspectRatio() const {
    int w, h;
    SDL_GetWindowSize(m_window, &w, &h);
    return (float)w / (float)h;
}

Vec2 Window::getSize() const {
    int w, h;
    SDL_GetWindowSize(m_window, &w, &h);
    return Vec2((float)w, (float)h);
}

void Window::setFullscreen(bool fullscreen) {
    SDL_SetWindowFullscreen(m_window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

bool Window::isFullscreen() const {
    Uint32 flags = SDL_GetWindowFlags(m_window);
    return (flags & SDL_WINDOW_FULLSCREEN) || (flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
}