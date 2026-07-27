#include "engine/input/imguiBridge.h"
#include "engine/input/input.h"
#include "engine/input/keycodes.h"
#include <imgui.h>

void SyncImGuiGamepad(const Input& input, ImGuiIO& io) {
    auto btn = [&](int engineButton, ImGuiKey key) {
        io.AddKeyEvent(key, input.isControllerButtonDown(engineButton));
    };

    btn(CONTROLLER_BUTTON_A,             ImGuiKey_GamepadFaceDown);
    btn(CONTROLLER_BUTTON_B,             ImGuiKey_GamepadFaceRight);
    btn(CONTROLLER_BUTTON_X,             ImGuiKey_GamepadFaceLeft);
    btn(CONTROLLER_BUTTON_Y,             ImGuiKey_GamepadFaceUp);
    btn(CONTROLLER_BUTTON_DPAD_UP,       ImGuiKey_GamepadDpadUp);
    btn(CONTROLLER_BUTTON_DPAD_DOWN,     ImGuiKey_GamepadDpadDown);
    btn(CONTROLLER_BUTTON_DPAD_LEFT,     ImGuiKey_GamepadDpadLeft);
    btn(CONTROLLER_BUTTON_DPAD_RIGHT,    ImGuiKey_GamepadDpadRight);
    btn(CONTROLLER_BUTTON_LEFTSHOULDER,  ImGuiKey_GamepadL1);
    btn(CONTROLLER_BUTTON_RIGHTSHOULDER, ImGuiKey_GamepadR1);
    btn(CONTROLLER_BUTTON_START,         ImGuiKey_GamepadStart);
    btn(CONTROLLER_BUTTON_BACK,          ImGuiKey_GamepadBack);
    btn(CONTROLLER_BUTTON_LEFTSTICK,     ImGuiKey_GamepadL3);
    btn(CONTROLLER_BUTTON_RIGHTSTICK,    ImGuiKey_GamepadR3);

    auto axisAsButton = [&](int engineAxis, bool positive, ImGuiKey key, float deadzone = 0.15f) {
        float v = input.getControllerAxis(engineAxis);
        float mag = positive ? v : -v;
        io.AddKeyAnalogEvent(key, mag > deadzone, mag > deadzone ? mag : 0.0f);
    };

    axisAsButton(CONTROLLER_AXIS_LEFTX, false, ImGuiKey_GamepadLStickLeft);
    axisAsButton(CONTROLLER_AXIS_LEFTX, true,  ImGuiKey_GamepadLStickRight);
    axisAsButton(CONTROLLER_AXIS_LEFTY, false, ImGuiKey_GamepadLStickUp);
    axisAsButton(CONTROLLER_AXIS_LEFTY, true,  ImGuiKey_GamepadLStickDown);
    axisAsButton(CONTROLLER_AXIS_TRIGGERLEFT,  true, ImGuiKey_GamepadL2);
    axisAsButton(CONTROLLER_AXIS_TRIGGERRIGHT, true, ImGuiKey_GamepadR2);
}