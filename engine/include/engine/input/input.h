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
    Vec2 getScrollDelta();

    void accumulateMouseDelta(int x, int y);
    void accumulateScrollDelta(float x, float y);
    void resetMouseDelta();
    void resetScrollDelta();

private:
    SDL_Window* m_window;

    std::vector<uint8_t> m_prevKeyState;
    std::vector<uint8_t> m_currKeyState;
    int m_mouseDeltaX = 0;
    int m_mouseDeltaY = 0;
    
    float m_scrollDeltaX = 0.0f;
    float m_scrollDeltaY = 0.0f;
};