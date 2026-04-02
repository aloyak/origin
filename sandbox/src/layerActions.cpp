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
            [this]() { 
                m_Engine.getSceneManager().createScene("Empty Scene");
                m_SelectedEntity = nullptr; 
                m_CurrentSceneInfo = { {}, {} }; 
            },
            nullptr,
            nullptr },

        { ActionSection::File, "Open Scene", "Ctrl+O", { KEY_O, true, false, false },
            [this]() { OpenScene(); },
            nullptr,
            nullptr },
        { ActionSection::File, "Open Recent", "Ctrl+Shift+O", { KEY_O, true, true, false }, // TODO
            [this]() { OpenSceneRecent(); },
            nullptr,
            nullptr },
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

        { ActionSection::File, "Quit", "Alt+F4", { 0, false, false, false },
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
            
        { ActionSection::Rendering, "Toggle Pixelart", "Ctrl+P", { KEY_P, true, false, false },
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

        { ActionSection::Preferences, "Show Render Stats", "F1", { KEY_F1, false, false, false },
            [this]() { m_ShowRenderStats = !m_ShowRenderStats; },
            nullptr,
            [this]() { return m_ShowRenderStats; } },
        /*{ ActionSection::Preferences, "Show Grid", "F2", { KEY_G, true, false, false },
            [this]() { ; },
            nullptr,
            [this]() { return} },*/
        { ActionSection::Preferences, "Clear User Preferences", nullptr, { 0, false, false, false },
            [this]() { ClearUserPreferences(); },
            nullptr,
            nullptr },
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
        { ActionSection::Physics, m_Engine.getPhysicsWorld().isEnabled() ? "Stop Physics" : "Play Physics", "P", { KEY_P, false, false, false },
            [this]() { 
                const bool wasEnabled = m_Engine.getPhysicsWorld().isEnabled();

                if (!wasEnabled) {
                    m_PhysicsEntities.clear();

                    for (auto& entityPtr : m_SceneManager.getActiveScene()->getEntities()) {
                        Entity* entity = entityPtr.get();
                        if (!entity->hasComponent<RigidbodyComponent>()) continue;
                        if (entity->getComponent<RigidbodyComponent>()->getBodyType() != RigidbodyComponent::BodyType::Dynamic) continue;
                        
                        m_PhysicsEntities.push_back({ entity, entity->transform.position, entity->transform.rotation });
                    }
                }

                m_Engine.getPhysicsWorld().setEnabled(!wasEnabled); 
            },
            [this]() { return m_SceneManager.getActiveScene() != nullptr; },
            [this]() { return m_Engine.getPhysicsWorld().isEnabled(); } },
        { ActionSection::Physics, "Reset Simulation", "R", { KEY_R, false, false, false },
            [this]() {
                m_Engine.getPhysicsWorld().setEnabled(false); // TODO: For now, check later
                for (const PhysicsEntityInfo& info : m_PhysicsEntities) {
                    info.entity->transform.position = info.initialPosition;
                    info.entity->transform.rotation = info.initialRotation;

                    if (info.entity->hasComponent<RigidbodyComponent>()) {
                        info.entity->getComponent<RigidbodyComponent>()->resetMotion();
                    }
                }
            },
            [this]() { return m_SceneManager.getActiveScene() != nullptr; },
            nullptr },
        { ActionSection::Physics, "Keep Simulation", "K", { KEY_K, false, false, false },
            [this]() { 
                for (PhysicsEntityInfo& info : m_PhysicsEntities) {
                    info.initialPosition = info.entity->transform.position;
                    info.initialRotation = info.entity->transform.rotation;
                }
            },
            [this]() { return m_SceneManager.getActiveScene() != nullptr; },
            nullptr },
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
            auto drawFileAction = [&](const char* label) {
                for (const ActionDef& action : actions) {
                    if (action.section == ActionSection::File && std::string(action.label) == label) {
                        DrawActionItem(action);
                        return;
                    }
                }
            };
            
            drawFileAction("New Scene");

            ImGui::Separator();

            drawFileAction("Open Scene");

            if (ImGui::BeginMenu("Open Recent")) {
                if (m_RecentScenes.empty()) {
                    ImGui::TextUnformatted("No recent scenes");
                } else {
                    for (const std::string& recentPath : m_RecentScenes) {
                        if (ImGui::MenuItem(recentPath.c_str())) {
                            OpenSceneFromPath(recentPath);
                        }
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            drawFileAction("Save Scene");
            drawFileAction("Save Scene As");
            drawFileAction("Unload Scene");

            ImGui::Separator();

            drawFileAction("Quit");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Preferences")) {
            ImGui::TextUnformatted("Camera Sensitivity");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(90.0f);
            if (ImGui::DragFloat("##Sensitivity", &m_CameraSens, 0.005f, 0.01f, 1.0f, "%.3f")) {
                SaveUserPreferences();
            }

            if (ImGui::BeginMenu("Color Theme")) {
                if (ImGui::MenuItem("Dark (Default)")) {
                    Styles::setupDarkTheme();
                }
                if (ImGui::MenuItem("Light")) {
                    ImGui::StyleColorsLight();
                }
                ImGui::EndMenu();
            }
            for (const ActionDef& action : actions) {
                if (action.section == ActionSection::Preferences && std::string(action.label) != "Clear User Preferences") {
                    DrawActionItem(action);
                }
            }
            ImGui::Separator();
            for (const ActionDef& action : actions) {
                if (action.section == ActionSection::Window) {
                    DrawActionItem(action);
                }
            }
            ImGui::Separator();
            for (const ActionDef& action : actions) {
                if (action.section == ActionSection::Preferences && std::string(action.label) == "Clear User Preferences") {
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
            if (ImGui::MenuItem("Duplicate Entity", "", false, m_SelectedEntity != nullptr)) {
                // Duplicated implementation from HierarchyPanel!
                Entity* newEntity = m_SceneManager.getActiveScene()->createEntity(m_SelectedEntity->name + " Copy");
                newEntity->transform = m_SelectedEntity->transform;
                for (const auto& [typeId, component] : m_SelectedEntity->getComponents()) {
                    newEntity->addComponentCopy(typeId, component);
                }
                m_SelectedEntity = newEntity;
            }
            ImGui::Separator();
            for (const ActionDef& action : actions) {
                if (action.section == ActionSection::Gizmos) {
                    DrawActionItem(action);
                }
            }
            ImGui::Separator();
            ImGui::SetNextItemWidth(150.0f);
            if (ImGui::SliderFloat("Editor Camera Speed", &m_CameraSpeed, 0.1f, 10.0f)) {
                SaveUserPreferences();
            }
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
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Physics")) {
            for (const ActionDef& action : actions) {
                if (action.section == ActionSection::Physics) {
                    DrawActionItem(action);
                }
            }

            ImGui::Separator();

            Vec3 gravity = m_Engine.getPhysicsWorld().getGravity();
            ImGui::TextUnformatted("Gravity");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(140.0f);
            if (ImGui::DragFloat3("##Gravity", &gravity.x, 0.1f)) {
                m_Engine.getPhysicsWorld().setGravity(gravity);
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
