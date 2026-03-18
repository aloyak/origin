#pragma once

#include "engine/engine.h"

#include <imgui.h>
#include <filesystem>
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
    void DrawMenuBar();
    void DrawDockspace();
    void DrawSceneView();
    void DrawHierarchy();
    void DrawProperties();

    ScenePathInfo GetSceneContext(const std::string& inputPath);

    Engine& m_Engine;
    Input& m_Input = m_Engine.getInput();
    Window& m_Window = m_Engine.getWindow();

    Entity* m_SelectedEntity = nullptr;
    ImVec2 m_ViewportSize = { 1600, 900 };

    void HandleCameraInput();

    Entity* m_EditorCamera = nullptr;
};