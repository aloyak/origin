#include "engine/input/input.h"
#include <SDL2/SDL.h>
#include <cstring>
#include <cmath>

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #include <emscripten/html5.h>

static int s_deltaX = 0;
static int s_deltaY = 0;

static EM_BOOL mouseMoveCallback(int, const EmscriptenMouseEvent* e, void*) {
    s_deltaX += e->movementX;
    s_deltaY += e->movementY;
    return EM_TRUE;
}
#endif

static constexpr float kDeviceSwitchAxisThreshold = 0.5f;

Input::Input(SDL_Window* window) : m_window(window) {
    SDL_PumpEvents();

    int numKeys = 0;
    const uint8_t* current = SDL_GetKeyboardState(&numKeys);

    m_prevKeyState.assign(current, current + numKeys);
    m_currKeyState.assign(current, current + numKeys);
#ifdef __EMSCRIPTEN__
    emscripten_set_mousemove_callback("#canvas", nullptr, 1, mouseMoveCallback);
#endif

    openFirstController();

    m_controllerAxisState.assign(SDL_CONTROLLER_AXIS_MAX, 0.0f);
    m_prevControllerButtonState.assign(SDL_CONTROLLER_BUTTON_MAX, 0);
    m_currControllerButtonState.assign(SDL_CONTROLLER_BUTTON_MAX, 0);
}

Input::~Input() {
    if (m_controller) {
        SDL_GameControllerClose(m_controller);
        m_controller = nullptr;
    }
}

void Input::openFirstController() {
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            m_controller = SDL_GameControllerOpen(i);
            if (m_controller) break;
        }
    }
}

void Input::update() {
    SDL_PumpEvents();

    int numKeys;
    const uint8_t* current = SDL_GetKeyboardState(&numKeys);

    if (m_currKeyState.size() < (size_t)numKeys) {
        m_currKeyState.resize(numKeys, 0);
    }
    if (m_prevKeyState.size() < (size_t)numKeys) {
        m_prevKeyState.resize(numKeys, 0);
    }

    memcpy(m_prevKeyState.data(), m_currKeyState.data(), numKeys);
    memcpy(m_currKeyState.data(), current, numKeys);

    bool keyboardActivity = false;
    for (int i = 0; i < numKeys; ++i) {
        if (m_currKeyState[i]) { keyboardActivity = true; break; }
    }
    bool mouseActivity = (m_mouseDeltaX != 0 || m_mouseDeltaY != 0) ||
                          (SDL_GetMouseState(nullptr, nullptr) != 0);

    updateController();

    bool controllerActivity = false;
    if (m_controller) {
        for (size_t i = 0; i < m_currControllerButtonState.size(); ++i) {
            if (m_currControllerButtonState[i]) { controllerActivity = true; break; }
        }
        if (!controllerActivity) {
            for (float axis : m_controllerAxisState) {
                if (std::fabs(axis) > kDeviceSwitchAxisThreshold) { controllerActivity = true; break; }
            }
        }
    }

    if (controllerActivity) {
        m_lastUsedDevice = InputMode::Controller;
    } else if (keyboardActivity || mouseActivity) {
        m_lastUsedDevice = InputMode::Keyboard;
    }
}

void Input::updateController() {
    if (!m_controller) return;

    m_prevControllerButtonState = m_currControllerButtonState;

    for (int b = 0; b < SDL_CONTROLLER_BUTTON_MAX; ++b) {
        m_currControllerButtonState[b] = SDL_GameControllerGetButton(m_controller, (SDL_GameControllerButton)b);
    }
    for (int a = 0; a < SDL_CONTROLLER_AXIS_MAX; ++a) {
        int16_t raw = SDL_GameControllerGetAxis(m_controller, (SDL_GameControllerAxis)a);
        m_controllerAxisState[a] = raw / 32767.0f;
    }
}

bool Input::isKeyPressed(int key) const {
    if (key < 0 || key >= (int)m_currKeyState.size() || key >= (int)m_prevKeyState.size()) return false;
    return m_currKeyState[key] && !m_prevKeyState[key];
}

bool Input::isKeyDown(int key) const {
    if (key < 0 || key >= (int)m_currKeyState.size()) return false;
    return m_currKeyState[key] != 0;
}

bool Input::isMouseButtonPressed(int button) const {
    return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button);
}

Vec2 Input::getMousePos() const {
    int x, y;
    SDL_GetMouseState(&x, &y);
    return Vec2((float)x, (float)y);
}

Vec2 Input::getMouseDelta() {
#ifdef __EMSCRIPTEN__
    Vec2 delta((float)s_deltaX, (float)s_deltaY);
    s_deltaX = 0;
    s_deltaY = 0;
    return delta;
#else
    Vec2 delta((float)m_mouseDeltaX, (float)m_mouseDeltaY);
    resetMouseDelta();
    return delta;
#endif
}

Vec2 Input::getScrollDelta() {
    Vec2 delta(m_scrollDeltaX, m_scrollDeltaY);
    resetScrollDelta();
    return delta;
}

void Input::resetMouseDelta() {
    m_mouseDeltaX = 0;
    m_mouseDeltaY = 0;
}

void Input::accumulateMouseDelta(int x, int y) {
    m_mouseDeltaX += x;
    m_mouseDeltaY += y;
}

void Input::accumulateScrollDelta(float x, float y) {
    m_scrollDeltaX += x;
    m_scrollDeltaY += y;
}

void Input::resetScrollDelta() {
    m_scrollDeltaX = 0.0f;
    m_scrollDeltaY = 0.0f;
}

void Input::setCursorMode(bool locked) {
#ifdef __EMSCRIPTEN__
    if (locked) {
        emscripten_set_click_callback("#canvas", nullptr, 1, [](int, const EmscriptenMouseEvent*, void*) -> EM_BOOL {
            emscripten_request_pointerlock("#canvas", 0);
            return EM_TRUE;
        });
    } else {
        emscripten_exit_pointerlock();
    }
#else
    SDL_SetRelativeMouseMode(locked ? SDL_TRUE : SDL_FALSE);
#endif
}

bool Input::isControllerButtonPressed(int button) const {
    if (!m_controller || button < 0 || button >= (int)m_currControllerButtonState.size()) return false;
    return m_currControllerButtonState[button] && !m_prevControllerButtonState[button];
}

bool Input::isControllerButtonDown(int button) const {
    if (!m_controller || button < 0 || button >= (int)m_currControllerButtonState.size()) return false;
    return m_currControllerButtonState[button] != 0;
}

float Input::getControllerAxis(int axis) const {
    if (!m_controller || axis < 0 || axis >= (int)m_controllerAxisState.size()) return 0.0f;
    return m_controllerAxisState[axis];
}

bool Input::isControllerConnected() const {
    return m_controller != nullptr;
}

InputMode Input::getActiveInputMode() const {
    return m_forcedMode == InputMode::Auto ? m_lastUsedDevice : m_forcedMode;
}

void Input::setInputMode(InputMode mode) {
    m_forcedMode = mode;
}