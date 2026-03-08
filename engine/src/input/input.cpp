#include "engine/input/input.h"
#include <SDL2/SDL.h>

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
#ifdef __EMSCRIPTEN__
    emscripten_set_mousemove_callback("#canvas", nullptr, 1, mouseMoveCallback);
#endif
}

bool Input::isKeyPressed(int key) const {
    return SDL_GetKeyboardState(NULL)[key] == SDL_PRESSED;
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

void Input::resetMouseDelta() {
    m_mouseDeltaX = 0;
    m_mouseDeltaY = 0;
}

void Input::accumulateMouseDelta(int x, int y) {
    m_mouseDeltaX += x;
    m_mouseDeltaY += y;
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