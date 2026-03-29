#include "sandbox/layer.h"

void Layer::DrawActionItem(const ActionDef& action) {
    const bool enabled = action.enabled ? action.enabled() : true;
    const bool checked = action.checked ? action.checked() : false;

    if (!enabled) {
        ImGui::BeginDisabled();
    }

    const bool activated = action.checked
        ? ImGui::MenuItem(action.label, action.shortcutLabel, checked)
        : ImGui::MenuItem(action.label, action.shortcutLabel);

    if (!enabled) {
        ImGui::EndDisabled();
    }

    if (activated && enabled) {
        action.execute();
    }
}

bool Layer::IsShortcutPressed(const Shortcut& shortcut) const {
    if (shortcut.key == 0) {
        return false;
    }

    const bool ctrlDown = m_Input.isKeyDown(KEY_LCTRL) || m_Input.isKeyDown(KEY_RCTRL);
    const bool shiftDown = m_Input.isKeyDown(KEY_LSHIFT) || m_Input.isKeyDown(KEY_RSHIFT);
    const bool altDown = m_Input.isKeyDown(KEY_LALT) || m_Input.isKeyDown(KEY_RALT);

    if (shortcut.ctrl && !ctrlDown) return false;
    if (shortcut.shift && !shiftDown) return false;
    if (shortcut.alt && !altDown) return false;

    return m_Input.isKeyPressed(shortcut.key);
}

Layer::ActionList Layer::BuildActions() {
    return {
        { ActionSection::File, "New Scene", "Ctrl+N", { KEY_N, true, false, false },
            [this]() { m_Engine.getSceneManager().createScene("Empty Scene"); },
            nullptr,
            nullptr },

        { ActionSection::File, "Open Scene", "Ctrl+O", { KEY_O, true, false, false },
            [this]() { OpenScene(); },
            nullptr,
            nullptr },

        // Keep this before Save Scene so Ctrl+Shift+S resolves to Save As.
        { ActionSection::File, "Save Scene As", "Ctrl+Shift+S", { KEY_S, true, true, false },
            [this]() { SaveSceneAs(); },
            [this]() { return m_SceneManager.getActiveScene() != nullptr; },
            nullptr },

        { ActionSection::File, "Save Scene", "Ctrl+S", { KEY_S, true, false, false },
            [this]() { SaveScene(); },
            [this]() { return m_SceneManager.getActiveScene() != nullptr; },
            nullptr },

        { ActionSection::File, "Unload Scene", nullptr, { 0, false, false, false },
            [this]() { UnloadScene(); },
            [this]() { return m_SceneManager.getActiveScene() != nullptr; },
            nullptr },

        { ActionSection::File, "Quit", nullptr, { 0, false, false, false },
            [this]() { m_Engine.stop(); },
            nullptr,
            nullptr },

        { ActionSection::Window, "Set Fullscreen", "F11", { KEY_F11, false, false, false },
            [this]() { m_Window.setFullscreen(!m_Window.isFullscreen()); },
            nullptr,
            [this]() { return m_Window.isFullscreen(); } },

        { ActionSection::Window, "Toggle VSync", nullptr, { 0, false, false, false },
            [this]() { m_Window.enableVSync(!m_Window.isVSyncEnabled()); },
            nullptr,
            [this]() { return m_Window.isVSyncEnabled(); } },

        { ActionSection::Scene, "Add Entity", "Ctrl+Shift+A", { KEY_A, true, true, false },
            [this]() {
                Entity* newEntity = m_SceneManager.getActiveScene()->createEntity("Entity");
                if (m_SelectedEntity) {
                    newEntity->transform.position = m_SelectedEntity->transform.position;
                }
                m_SelectedEntity = newEntity;
            },
            [this]() { return m_SceneManager.getActiveScene() != nullptr; },
            nullptr },

        { ActionSection::Scene, "Delete Entity", "Del", { KEY_DELETE, false, false, false },
            [this]() {
                m_SceneManager.getActiveScene()->destroyEntity(m_SelectedEntity);
                m_SelectedEntity = nullptr;
            },
            [this]() { return m_SelectedEntity != nullptr; },
            nullptr },

        { ActionSection::Rendering, "Toggle Pixelart", nullptr, { 0, false, false, false },
            [this]() {
                if (!m_Renderer.isPixelArtEnabled()) {
                    m_Renderer.setupRenderTarget(350, 200);
                    m_Renderer.setPixelArt(true, 4);
                } else {
                    Vec2 size = m_Window.getSize();
                    m_Renderer.setupRenderTarget((unsigned int)size.x, (unsigned int)size.y);
                    m_Renderer.setPixelArt(false, 32);
                }
            },
            nullptr,
            [this]() { return m_Renderer.isPixelArtEnabled(); } },

        { ActionSection::Rendering, "Toggle Vertex Snap", "Ctrl+V", { KEY_V, true, false, false },
            [this]() { m_Renderer.setVertexSnap(!m_Renderer.isVertexSnapEnabled()); },
            nullptr,
            [this]() { return m_Renderer.isVertexSnapEnabled(); } },

        { ActionSection::Rendering, "Toggle Lighting", "Ctrl+L", { KEY_L, true, false, false },
            [this]() { m_Renderer.setLightingEnabled(!m_Renderer.isLightingEnabled()); },
            nullptr,
            [this]() { return m_Renderer.isLightingEnabled(); } },

        { ActionSection::Rendering, "Show Render Stats", "F1", { KEY_F1, false, false, false },
            [this]() { m_ShowRenderStats = !m_ShowRenderStats; },
            nullptr,
            [this]() { return m_ShowRenderStats; } },

        { ActionSection::Gizmos, "Translate", "1", { KEY_1, false, false, false },
            [this]() { m_GizmoOperation = ImGuizmo::OPERATION::TRANSLATE; },
            nullptr,
            [this]() { return m_GizmoOperation == ImGuizmo::OPERATION::TRANSLATE; } },

        { ActionSection::Gizmos, "Rotate", "2", { KEY_2, false, false, false },
            [this]() { m_GizmoOperation = ImGuizmo::OPERATION::ROTATE; },
            nullptr,
            [this]() { return m_GizmoOperation == ImGuizmo::OPERATION::ROTATE; } },

        { ActionSection::Gizmos, "Scale", "3", { KEY_3, false, false, false },
            [this]() { m_GizmoOperation = ImGuizmo::OPERATION::SCALE; },
            nullptr,
            [this]() { return m_GizmoOperation == ImGuizmo::OPERATION::SCALE; } },

        { ActionSection::Gizmos, "Universal", "4", { KEY_4, false, false, false },
            [this]() { m_GizmoOperation = ImGuizmo::OPERATION::UNIVERSAL; },
            nullptr,
            [this]() { return m_GizmoOperation == ImGuizmo::OPERATION::UNIVERSAL; } },

        { ActionSection::Help, "About", nullptr, { 0, false, false, false },
            [this]() {
                if (!m_AboutPanel) {
                    m_AboutPanel = new AboutPanel();
                    m_Panels.push_back(std::unique_ptr<Panel>(m_AboutPanel));
                }
                m_AboutPanel->Open();
            },
            nullptr,
            nullptr },

        { ActionSection::Help, "GitHub", nullptr, { 0, false, false, false },
            [this]() {
                std::string url = "https://github.com/aloyak/origin";

                #ifdef _WIN32
                    ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWDEFAULT);
                #elif __APPLE__
                    std::string command = "open " + url;
                    system(command.c_str());
                #else
                    std::string command = "xdg-open " + url;
                    system(command.c_str());
                #endif
            },
            nullptr,
            nullptr },
    };
}

void Layer::DrawMenuBar() {
    const ActionList actions = BuildActions();
    HandleShortcuts();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            for (const ActionDef& action : actions) {
                if (action.section == ActionSection::File && std::string(action.label) == "New Scene") {
                    DrawActionItem(action);
                }
            }
            ImGui::Separator();

            for (const ActionDef& action : actions) {
                if (action.section == ActionSection::File && std::string(action.label) != "New Scene" && std::string(action.label) != "Quit") {
                    DrawActionItem(action);
                }
            }

            ImGui::Separator();
            for (const ActionDef& action : actions) {
                if (action.section == ActionSection::File && std::string(action.label) == "Quit") {
                    DrawActionItem(action);
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Preferences")) {
            ImGui::TextUnformatted("Camera Sensitivity");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(90.0f);
            ImGui::DragFloat("##Sensitivity", &m_CameraSens, 0.005f, 0.01f, 1.0f, "%.3f");

            if (ImGui::BeginMenu("Themes")) {
                if (ImGui::MenuItem("Dark")) {
                    Styles::setupDarkTheme();
                }
                if (ImGui::MenuItem("Light")) {
                    ImGui::StyleColorsLight();
                }
                ImGui::EndMenu();
            }

            ImGui::SeparatorText("Window");
            for (const ActionDef& action : actions) {
                if (action.section == ActionSection::Window) {
                    DrawActionItem(action);
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Scene")) {
            char sceneBuffer[256] = "";
            if (m_SceneManager.getActiveScene()) {
                strncpy(sceneBuffer, m_SceneManager.getActiveScene()->name.c_str(), 255);
            }

            if (!m_SceneManager.getActiveScene()) ImGui::BeginDisabled();
            ImGui::TextUnformatted("Scene Name");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(200.0f);
            if (ImGui::InputText("##sceneName", sceneBuffer, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (m_SceneManager.getActiveScene()) m_SceneManager.getActiveScene()->name = sceneBuffer;
            }
            if (!m_SceneManager.getActiveScene()) ImGui::EndDisabled();
            ImGui::Separator();
            for (const ActionDef& action : actions) {
                if (action.section == ActionSection::Scene) {
                    DrawActionItem(action);
                }
            }
            if (ImGui::MenuItem("Duplicate Entity", "", false, m_SelectedEntity != nullptr)) {}
            ImGui::Separator();
            for (const ActionDef& action : actions) {
                if (action.section == ActionSection::Gizmos) {
                    DrawActionItem(action);
                }
            }
            ImGui::Separator();
            ImGui::SetNextItemWidth(150.0f);
            ImGui::SliderFloat("Editor Camera Speed", &m_CameraSpeed, 0.1f, 10.0f);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Rendering")) {
            for (const ActionDef& action : actions) {
                if (action.section == ActionSection::Rendering && std::string(action.label) != "Show Render Stats") {
                    DrawActionItem(action);
                }
            }
            ImGui::Separator();
            float ambient = m_Renderer.getMinimumAmbientLight();
            ImGui::TextUnformatted("Ambient Light");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(70.0f);
            if (ImGui::DragFloat("##AmbientLight", &ambient, 0.01f, 0.0f)) {
                m_Renderer.setMinimumAmbientLight(ambient);
            }
            ImGui::Separator();
            for (const ActionDef& action : actions) {
                if (action.section == ActionSection::Rendering && std::string(action.label) == "Show Render Stats") {
                    DrawActionItem(action);
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            for (const ActionDef& action : actions) {
                if (action.section == ActionSection::Help) {
                    DrawActionItem(action);
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Layer::HandleShortcuts() {
    const ActionList actions = BuildActions();

    for (const ActionDef& action : actions) {
        if (!IsShortcutPressed(action.shortcut)) continue;

        const bool enabled = action.enabled ? action.enabled() : true;
        if (!enabled) continue;

        action.execute();
        break;
    }
}
