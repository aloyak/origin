#pragma once

#include "engine/engine.h"
#include "engine/core/math.h"
#include "engine/utils/logger.h"
#include "engine/components/rigidbodyComponent.h"

#include "sandbox/panel/panel.h"
#include "sandbox/panel/aboutPanel.h"
#include "sandbox/styles.h"

#include <imgui.h>
#include <ImGuizmo.h>
#include <filesystem>
#include <vector>
#include <memory>
#include <functional>
#include <string>

namespace fs = std::filesystem;

struct ScenePathInfo {
    fs::path root;
    fs::path scene;
};

class Layer {
public:
    Layer(Engine& engine);
    void OnUIRender();

private:
    struct Shortcut {
        int key = KEY_NONE;
        bool ctrl = false;
        bool shift = false;
        bool alt = false;
    };

    enum class ActionSection {
        File,
        Preferences,
        Window,
        Scene,
        Rendering,
        Gizmos,
        Physics,
        Help,
    };

    struct ActionDef {
        ActionSection section;
        const char* label;
        const char* shortcutLabel;
        Shortcut shortcut;
        std::function<void()> execute;
        std::function<bool()> enabled;
        std::function<bool()> checked;
    };

    using ActionList = std::vector<ActionDef>;

    void DrawMenuBar();
    void DrawDockspace();
    ScenePathInfo GetSceneContext(const std::string& inputPath);

    ActionList BuildActions();
    void DrawActionItem(const ActionDef& action);
    bool IsShortcutPressed(const Shortcut& shortcut) const;
    void HandleShortcuts();

    Engine& m_Engine;
    Renderer& m_Renderer = m_Engine.getRenderer();
    Window& m_Window = m_Engine.getWindow();
    SceneManager& m_SceneManager = m_Engine.getSceneManager();
    Input &m_Input = m_Engine.getInput();

    Entity* m_SelectedEntity = nullptr;
    Entity* m_EditorCamera = nullptr;
    float m_CameraSpeed = 1.0f;
    float m_CameraSens = 0.15f;
    ImGuizmo::OPERATION m_GizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
    
    std::vector<std::unique_ptr<Panel>> m_Panels;

    ImFont* m_RegularFont = nullptr;
    ImFont* m_SemiBoldFont = nullptr;
    ImFont* m_ExtraBoldFont = nullptr;

    bool m_ShowRenderStats = true;

    // Scene management
    void OpenScene();
    void OpenSceneRecent();
    void SaveScene();
    void SaveSceneAs();
    void UnloadScene();

    bool OpenSceneFromPath(const fs::path& inputPath);

    // User preferences
    void LoadUserPreferences();
    void SaveUserPreferences() const;
    void ClearUserPreferences();
    void AddRecentScene(const fs::path& scenePath);

    void registerDefaultInspectors();

    // Physics (play/reset simulation)
    struct PhysicsEntityInfo {
        Entity* entity;
        Vec3 initialPosition;
        Vec3 initialRotation;
    };
    std::vector<PhysicsEntityInfo> m_PhysicsEntities;

    ScenePathInfo m_CurrentSceneInfo;
    fs::path m_UserPreferencesPath;
    std::vector<std::string> m_RecentScenes;

    // Closeable panels
    AboutPanel* m_AboutPanel = nullptr;
};