#pragma once

#include "engine/input/keycodes.h"
#include "engine/math.h"

struct SDL_Window;

class Input {
public:
    Input(SDL_Window* window);

    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;

    void setCursorMode(bool locked);

    Vec2 getMousePos() const;
    Vec2 getMouseDelta();

    void accumulateMouseDelta(int x, int y);
    void resetMouseDelta();

private:
    SDL_Window* m_window;

    int m_mouseDeltaX = 0;
    int m_mouseDeltaY = 0;
};