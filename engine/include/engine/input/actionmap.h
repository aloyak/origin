#pragma once

#include "engine/input/input.h"
#include "engine/input/inputTypes.h"
#include "engine/input/keycodes.h"
#include "engine/core/userSettings.h"

#include <string>

class ActionMap {
public:
    explicit ActionMap(Input& input, const std::string& settingsPath = "user/keybinds.json");

    void registerAction(const std::string& name, ActionBinding defaultBinding);

    bool isActionPressed(const std::string& name) const;
    bool isActionDown(const std::string& name) const;
    float getActionAxis(const std::string& name) const;

    void bindKey(const std::string& name, int key);
    void bindControllerButton(const std::string& name, int button);
    void bindControllerAxis(const std::string& name, int axis, bool positive = true);

    const ActionBinding* getBinding(const std::string& name) const;


    void save();
    void load();

    InputMode getActiveInputMode() const { return m_input.getActiveInputMode(); }
    void setInputMode(InputMode mode) { m_input.setInputMode(mode); }

    Input& GetInput() { return m_input; }
    const Input& GetInput() const { return m_input; }
private:
    Input& m_input;
    SettingsManager<ActionBindings> m_settings;
};