#include "engine/input/actionmap.h"
#include <cmath>

ActionMap::ActionMap(Input& input, const std::string& settingsPath)
    : m_input(input), m_settings(settingsPath) {}

void ActionMap::registerAction(const std::string& name, ActionBinding defaultBinding) {
    auto& actions = m_settings.Get().actions;
    if (actions.find(name) == actions.end()) {
        actions[name] = defaultBinding;
    }
}

bool ActionMap::isActionDown(const std::string& name) const {
    const ActionBinding* b = getBinding(name);
    if (!b) return false;

    if (b->key != 0 && m_input.isKeyDown(b->key)) return true;
    if (b->mouseButton >= 0 && m_input.isMouseButtonDown(b->mouseButton)) return true;
    if (b->controllerButton >= 0 && m_input.isControllerButtonDown(b->controllerButton)) return true;
    if (b->controllerAxis >= 0) {
        float v = m_input.getControllerAxis(b->controllerAxis);
        if (b->axisPositive && v > b->axisDeadzone) return true;
        if (!b->axisPositive && v < -b->axisDeadzone) return true;
    }
    return false;
}

bool ActionMap::isActionPressed(const std::string& name) const {
    const ActionBinding* b = getBinding(name);
    if (!b) return false;

    if (b->key != 0 && m_input.isKeyPressed(b->key)) return true;
    if (b->mouseButton >= 0 && m_input.isMouseButtonJustPressed(b->mouseButton)) return true;
    if (b->controllerButton >= 0 && m_input.isControllerButtonPressed(b->controllerButton)) return true;

    return false;
}

float ActionMap::getActionAxis(const std::string& name) const {
    const ActionBinding* b = getBinding(name);
    if (!b || b->controllerAxis < 0) return 0.0f;

    float v = m_input.getControllerAxis(b->controllerAxis);
    if (std::fabs(v) < b->axisDeadzone) return 0.0f;
    return v;
}

void ActionMap::bindKey(const std::string& name, int key) {
    m_settings.Get().actions[name].key = key;
}

void ActionMap::bindMouseButton(const std::string& name, int button) {
    m_settings.Get().actions[name].mouseButton = button;
}

void ActionMap::bindControllerButton(const std::string& name, int button) {
    m_settings.Get().actions[name].controllerButton = button;
}

void ActionMap::bindControllerAxis(const std::string& name, int axis, bool positive) {
    auto& b = m_settings.Get().actions[name];
    b.controllerAxis = axis;
    b.axisPositive = positive;
}

const ActionBinding* ActionMap::getBinding(const std::string& name) const {
    const auto& actions = m_settings.Get().actions;
    auto it = actions.find(name);
    return it == actions.end() ? nullptr : &it->second;
}

void ActionMap::save() {
    m_settings.Save();
}

void ActionMap::load() {
    m_settings.Load();
}