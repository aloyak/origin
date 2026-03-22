#pragma once
#include "engine/engine.h"

#include "sandbox/panel/panel.h"
#include "sandbox/panel/aboutPanel.h"

#include <imgui.h>
#include <filesystem>
#include <vector>
#include <memory>

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
    void DrawMenuBar();
    void DrawDockspace();
    ScenePathInfo GetSceneContext(const std::string& inputPath);

    void HandleShortcuts();

    Engine& m_Engine;
    Renderer& m_Renderer = m_Engine.getRenderer();
    Window& m_Window = m_Engine.getWindow();
    SceneManager& m_SceneManager = m_Engine.getSceneManager();
    Input &m_Input = m_Engine.getInput();

    Entity* m_SelectedEntity = nullptr;
    Entity* m_EditorCamera = nullptr;
    
    std::vector<std::unique_ptr<Panel>> m_Panels;

    ImFont* m_RegularFont = nullptr;
    ImFont* m_SemiBoldFont = nullptr;
    ImFont* m_ExtraBoldFont = nullptr;

    // Scene management
    void OpenScene();
    void SaveScene();
    void SaveSceneAs();
    void UnloadScene();

    ScenePathInfo m_CurrentSceneInfo;

    // Closeable panels
    AboutPanel* m_AboutPanel = nullptr;
};