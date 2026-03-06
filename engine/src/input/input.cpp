#include "engine/input/input.h"
#include <SDL2/SDL.h>

Input::Input(SDL_Window* window) : m_window(window) {}

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
    int x, y;
    SDL_GetRelativeMouseState(&x, &y);
    return Vec2((float)x, (float)y);
}

void Input::setCursorMode(bool locked) {
    SDL_SetRelativeMouseMode(locked ? SDL_TRUE : SDL_FALSE);
}