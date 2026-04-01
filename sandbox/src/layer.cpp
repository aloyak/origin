#include "sandbox/layer.h"
#include "sandbox/inspectorRegistry.h"
#include "sandbox/panel/hierarchyPanel.h"
#include "sandbox/panel/propertiesPanel.h"
#include "sandbox/panel/sceneViewPanel.h"
#include "sandbox/dialog.h"

#include "engine/components/cameraComponent.h"

#include "engine/input/input.h"
#include "engine/utils/path.h"
#include "engine/utils/logger.h"

#include "sandbox/styles.h"

#include <iostream>

Layer::Layer(Engine& engine) 
    : m_Engine(engine) {
    
    ImGuiIO& io = m_Engine.getIO();
    static std::string layout = Path::resolve("resources/layout.ini").string();
    io.IniFilename = layout.c_str();

    // Styles
    ImGui::StyleColorsDark();
    Styles::setupDarkTheme();

    float fontSize = 15.0f;
    m_RegularFont = io.Fonts->AddFontFromFileTTF(Path::resolve("resources/Inter-4.1/Inter-Regular.ttf").string().c_str(), fontSize);
    m_SemiBoldFont = io.Fonts->AddFontFromFileTTF(Path::resolve("resources/Inter-4.1/Inter-SemiBold.ttf").string().c_str(), fontSize + 2.0f);
    m_ExtraBoldFont = io.Fonts->AddFontFromFileTTF(Path::resolve("resources/Inter-4.1/Inter-ExtraBold.ttf").string().c_str(), fontSize + 4.0f);

    io.FontDefault = m_RegularFont;
    io.Fonts->Build();

    registerDefaultInspectors();

    m_Engine.getPhysicsWorld().setEnabled(false);

    Entity* cam = m_Engine.createEntity("Editor Camera");
    cam->addComponent<CameraComponent>(60.0f, m_Window.getAspectRatio(), 0.1f, 10000.0f);
    cam->transform.position = Vec3(0.0f, 150.0f, 0.0f);
    m_EditorCamera = cam;

    m_Panels.push_back(std::make_unique<HierarchyPanel>(m_Engine, m_SelectedEntity));
    m_Panels.push_back(std::make_unique<PropertiesPanel>(m_Engine, m_SelectedEntity));
    m_Panels.push_back(std::make_unique<SceneViewPanel>(m_Engine, m_EditorCamera, m_CameraSpeed, m_SelectedEntity, m_GizmoOperation, m_ShowRenderStats, m_CameraSens));
}

void Layer::OnUIRender() {
    DrawMenuBar();
    DrawDockspace();
    
    // DEBUG!! (styling)
    //ImGui::ShowStyleEditor();

    for (auto& panel : m_Panels) {
        panel->OnUIRender();
    }

    const std::string title = m_SceneManager.getActiveScene() 
    ? "Origin Sandbox - " + m_SceneManager.getActiveScene()->name 
    : "Origin Sandbox";
    m_Window.setWindowTitle(title.c_str());
}

void Layer::OpenScene() {
    std::string path = Dialog::openFile({ "Scene Files", "*.json" });
    if (!path.empty() && path != m_CurrentSceneInfo.scene.string()) {
        ScenePathInfo candidateContext = GetSceneContext(path);

        if (candidateContext.scene.empty()) {
            Logger::error("Failed to load scene: Invalid path.");
            return; 
        }

        std::string oldBase = Path::getBase();
        Path::setBase(candidateContext.root);

        Scene* loaded = m_Engine.getSceneManager().load(candidateContext.scene.string());
        if (loaded) {
            m_CurrentSceneInfo = candidateContext;
            m_SelectedEntity = nullptr; 
        } else {
            Logger::error("Failed to load scene: " + candidateContext.scene.string());
            Path::setBase(oldBase); 
        }
    }
}

void Layer::SaveScene() {
    if (!m_SceneManager.getActiveScene()) {
        Logger::error("No active scene to save.");
        return;
    }
    
    if (m_CurrentSceneInfo.scene.empty()) { 
        SaveSceneAs();
        return;
    }
    
    m_Engine.getSceneManager().save(m_CurrentSceneInfo.scene.string());
}

void Layer::SaveSceneAs() {
    if (!m_SceneManager.getActiveScene()) {
        Logger::error("No active scene to save.");
        return;
    }

    std::string path = Dialog::saveFile({ "Scene Files", "*.json" });
    if (!path.empty()) {
        fs::path chosenPath(path);
        if (!chosenPath.has_extension()) {
            chosenPath.replace_extension(".json");
        }

        chosenPath = fs::absolute(chosenPath).lexically_normal();
        m_CurrentSceneInfo = { chosenPath.parent_path(), chosenPath };

        m_Engine.getSceneManager().save(chosenPath.string());
    }
}

void Layer::UnloadScene() {
    m_Engine.getSceneManager().unload();
    m_SelectedEntity = nullptr;
    m_CurrentSceneInfo = { {}, {} };
}

void Layer::DrawDockspace() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    float menuBarHeight = ImGui::GetFrameHeight();

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - menuBarHeight));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##dockspace", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground);
    ImGui::PopStyleVar();

    ImGui::DockSpace(ImGui::GetID("MainDockspace"), ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();
}

ScenePathInfo Layer::GetSceneContext(const std::string& inputPath) {
    fs::path fullPath = fs::absolute(inputPath).lexically_normal();
    std::string pathStr = fullPath.generic_string();

    size_t pos = pathStr.find("/assets/");
    if (pos == std::string::npos) {
        Logger::error(std::format("Invalid scene path: '{}'. Must be inside the 'assets' directory.", inputPath)); 
        return { {}, {} };
    }

    return { fs::path(pathStr.substr(0, pos + 1)), fullPath };
}