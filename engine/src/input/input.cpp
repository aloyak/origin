#include "engine/input/input.h"
#include <SDL2/SDL.h>
#include <cstring>

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

Input::Input(SDL_Window* window) : m_window(window) {
    SDL_PumpEvents();

    int numKeys = 0;
    const uint8_t* current = SDL_GetKeyboardState(&numKeys);

    m_prevKeyState.assign(current, current + numKeys);
    m_currKeyState.assign(current, current + numKeys);
#ifdef __EMSCRIPTEN__
    emscripten_set_mousemove_callback("#canvas", nullptr, 1, mouseMoveCallback);
#endif
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