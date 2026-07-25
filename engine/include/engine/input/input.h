#pragma once

#include "engine/input/keycodes.h"
#include "engine/input/inputTypes.h"
#include "engine/core/math.h"
#include <vector>
#include <cstdint>

struct SDL_Window;
struct _SDL_GameController;
typedef struct _SDL_GameController SDL_GameController;

#define CURSOR_LOCKED 1
#define CURSOR_DEFAULT 0

class Input {
public:
    Input(SDL_Window* window);
    ~Input();

    void update();
    bool isKeyPressed(int key) const;
    bool isKeyDown(int key) const;
    bool isMouseButtonPressed(int button) const;

    void setCursorMode(bool locked);

    void setMousePos(float x, float y);
    Vec2 getMousePos() const;
    Vec2 getMouseDelta();
    Vec2 getScrollDelta();

    void accumulateMouseDelta(int x, int y);
    void accumulateScrollDelta(float x, float y);
    void resetMouseDelta();
    void resetScrollDelta();

    
    bool isControllerButtonPressed(int button) const;
    bool isControllerButtonDown(int button) const;
    float getControllerAxis(int axis) const;
    bool isControllerConnected() const;

    int getAnyKeyPressed() const;
    int getAnyControllerButtonPressed() const;


    InputMode getLastUsedDevice() const { return m_lastUsedDevice; }
    InputMode getActiveInputMode() const;
    void setInputMode(InputMode mode);

private:
    void updateController();
    void openFirstController();
    void refreshControllerConnection();

    SDL_Window* m_window;
    SDL_GameController* m_controller = nullptr;

    std::vector<uint8_t> m_prevKeyState;
    std::vector<uint8_t> m_currKeyState;

    std::vector<uint8_t> m_prevControllerButtonState;
    std::vector<uint8_t> m_currControllerButtonState;
    std::vector<float> m_controllerAxisState;

    int m_mouseDeltaX = 0;
    int m_mouseDeltaY = 0;

    float m_scrollDeltaX = 0.0f;
    float m_scrollDeltaY = 0.0f;

    InputMode m_lastUsedDevice = InputMode::Keyboard;
    InputMode m_forcedMode = InputMode::Auto;
};