#pragma once

#include "engine/input/keycodes.h"
#include "engine/core/math.h"
#include <vector>
#include <cstdint>

struct SDL_Window;

class Input {
public:
    Input(SDL_Window* window);

    void update();
    bool isKeyPressed(int key) const;
    bool isKeyDown(int key) const;
    bool isMouseButtonPressed(int button) const;

    void setCursorMode(bool locked);

    Vec2 getMousePos() const;
    Vec2 getMouseDelta();

    void accumulateMouseDelta(int x, int y);
    void resetMouseDelta();

private:
    SDL_Window* m_window;

    std::vector<uint8_t> m_prevKeyState;
    int m_mouseDeltaX = 0;
    int m_mouseDeltaY = 0;
};