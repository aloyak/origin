#pragma once

#include <SDL2/SDL.h>

#include "engine/math.h"

class Window {
public:
    Window(unsigned int width, unsigned int height, const char* title);
    ~Window();

    void swapBuffers();

    void setFullscreen(bool fullscreen);
    void setWindowTitle(const char* title);
    void enableVSync(bool enabled);

    float getAspectRatio() const;
    Vec2 getSize() const;
    bool isFullscreen() const;

    SDL_Window* getHandle() const { return m_window; }
    void* getGLContext() const { return m_glContext; }

private:
    SDL_Window* m_window;
    SDL_GLContext m_glContext;
};