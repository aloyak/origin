#pragma once

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

enum class InputMode {
    Keyboard,
    Controller,
    Auto
};

struct ActionBinding {
    int key = 0;
    int controllerButton = -1;
    int controllerAxis = -1;
    bool axisPositive = true;
    float axisDeadzone = 0.1f;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ActionBinding, key, controllerButton, controllerAxis, axisPositive, axisDeadzone)
};

struct ActionBindings {
    std::unordered_map<std::string, ActionBinding> actions;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ActionBindings, actions)
};