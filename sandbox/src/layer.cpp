#include "sandbox/layer.h"
#include "sandbox/inspectorRegistry.h"

#include "sandbox/panel/hierarchyPanel.h"
#include "sandbox/panel/propertiesPanel.h"
#include "sandbox/panel/sceneViewPanel.h"
#include "sandbox/panel/statusPanel.h"

#include "sandbox/dialog.h"

#include "engine/components/cameraComponent.h"

#include "engine/input/input.h"
#include "engine/utils/path.h"
#include "engine/utils/logger.h"

#include "sandbox/styles.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>

namespace {
constexpr std::size_t kMaxRecentScenes = 5;
}

void Layer::ApplyAudioSettings() {
    const float appliedVolume = m_AudioEnabled ? std::clamp(m_MasterVolume, 0.0f, 8.0f) : 0.0f;
    m_Engine.getAudioSystem().setGlobalVolume(appliedVolume);
}

Layer::Layer(Engine& engine) : m_Engine(engine) {
    
    ImGuiIO& io = m_Engine.getIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    static std::string layout = Path::resolve("resources/layout.ini").string();
    io.IniFilename = layout.c_str();
    m_UserPreferencesPath = Path::resolve("resources/user.json");

    // Styles
    ImGui::StyleColorsDark();
    Styles::setupDarkTheme();


    LoadUserPreferences();
    ApplyAudioSettings();

    m_Window.setSize((unsigned int)m_WindowSize.x, (unsigned int)m_WindowSize.y);

    SetupFonts();

    registerDefaultInspectors();

    m_Engine.getPhysicsWorld().setEnabled(false);

    Entity* cam = m_Engine.createEntity("Editor Camera");
    cam->addComponent<CameraComponent>(60.0f, m_Window.getAspectRatio(), 0.1f, 10000.0f);
    cam->transform.position = Vec3(0.0f, 0.0f, 0.0f);
    m_EditorCamera = cam;
    m_EditorCamera->addComponent<ListenerComponent>();

    m_Panels.push_back(std::make_unique<HierarchyPanel>(m_Engine, m_SelectedEntity));
    m_Panels.push_back(std::make_unique<PropertiesPanel>(m_Engine, m_SelectedEntity));
    m_Panels.push_back(std::make_unique<SceneViewPanel>(m_Engine, m_EditorCamera, m_CameraSpeed, m_SelectedEntity, m_ColliderDebugEntities, m_GizmoOperation, m_ShowRenderStats, m_CameraSens));
    auto statusPanel = std::make_unique<StatusPanel>();
    m_StatusPanel = statusPanel.get();
    m_Panels.push_back(std::move(statusPanel));

    Logger::setCallback([this](const std::string& message) {
        if (m_StatusPanel) m_StatusPanel->SetConsole(message);
    });
}

Layer::~Layer() {
    SaveUserPreferences();
}

void Layer::SetupFonts() {
    ImGuiIO& io = m_Engine.getIO();
    io.Fonts->Clear();

    float fontSize = 15.0f;
    const std::string iconFontPath = Path::resolve("resources/lucide-1.7.0/" FONT_ICON_FILE_NAME_LC).string();

    static const ImWchar iconRange[] = { ICON_MIN_LC, ICON_MAX_16_LC, 0 };

    ImFontConfig textConfig;
    textConfig.GlyphExcludeRanges = iconRange;

    m_RegularFont = io.Fonts->AddFontFromFileTTF(
        Path::resolve("resources/Inter-4.1/Inter-Regular.ttf").string().c_str(),
        fontSize,
        &textConfig,
        io.Fonts->GetGlyphRangesDefault()
    );
    m_SemiBoldFont = io.Fonts->AddFontFromFileTTF(
        Path::resolve("resources/Inter-4.1/Inter-SemiBold.ttf").string().c_str(),
        fontSize + 2.0f,
        &textConfig,
        io.Fonts->GetGlyphRangesDefault()
    );
    m_ExtraBoldFont = io.Fonts->AddFontFromFileTTF(
        Path::resolve("resources/Inter-4.1/Inter-ExtraBold.ttf").string().c_str(),
        fontSize + 4.0f,
        &textConfig,
        io.Fonts->GetGlyphRangesDefault()
    );

    float iconFontSize = fontSize * 0.8f;
    auto mergeIconsInto = [&](ImFont* dstFont) {
        if (!dstFont) {
            return;
        }

        ImFontConfig iconConfig;
        iconConfig.MergeMode = true;
        iconConfig.DstFont = dstFont;
        iconConfig.PixelSnapH = true;
        iconConfig.GlyphMinAdvanceX = iconFontSize;

        io.Fonts->AddFontFromFileTTF(
            iconFontPath.c_str(),
            iconFontSize,
            &iconConfig,
            iconRange
        );
    };

    mergeIconsInto(m_RegularFont);
    mergeIconsInto(m_SemiBoldFont);
    mergeIconsInto(m_ExtraBoldFont);

    io.FontDefault = m_RegularFont;
    
    io.Fonts->Build();
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
    std::string path = Dialog::openFile({ "Scene Files", "*.json *.scene" });
    if (!path.empty()) {
        OpenSceneFromPath(path);
    }
}

void Layer::OpenLastScene() {
    if (m_RecentScenes.empty()) {
        Logger::warn("No last scene in user preferences.");
        return;
    }

    OpenSceneFromPath(m_RecentScenes.front());
}

void Layer::OpenSceneRecent() {
    if (m_RecentScenes.empty()) {
        Logger::warn("No recent scenes in user preferences.");
        return;
    }

    for (const std::string& recentPath : m_RecentScenes) {
        if (OpenSceneFromPath(recentPath)) {
            return;
        }
    }

    Logger::error("Failed to load any scene from recent list.");
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
    
    m_SceneManager.save(m_CurrentSceneInfo.scene.string());
    AddRecentScene(m_CurrentSceneInfo.scene);
}

void Layer::SaveSceneAs() {
    if (!m_SceneManager.getActiveScene()) {
        Logger::error("No active scene to save.");
        return;
    }

    std::string path = Dialog::saveFile({ "Scene Files", "*.json *.scene" });
    if (!path.empty()) {
        fs::path chosenPath(path);
        if (!chosenPath.has_extension()) {
            chosenPath.replace_extension(".json");
        }

        chosenPath = fs::absolute(chosenPath).lexically_normal();
        m_CurrentSceneInfo = { chosenPath.parent_path(), chosenPath };

        m_SceneManager.save(chosenPath.string());
        AddRecentScene(chosenPath);
    }
}

void Layer::UnloadScene() {
    m_ColliderDebugEntities.clear();
    m_SceneManager.unload();
    m_SelectedEntity = nullptr;
    m_CurrentSceneInfo = { {}, {} };
}

bool Layer::OpenSceneFromPath(const fs::path& inputPath) {
    ScenePathInfo candidateContext = GetSceneContext(inputPath.string());

    if (candidateContext.scene.empty()) {
        Logger::error("Failed to load scene: Invalid path.");
        return false;
    }

    fs::path oldBase = Path::getBase();
    Path::setBase(candidateContext.root);

    // Disable Physics before loading
    m_Engine.getPhysicsWorld().setEnabled(false);
    m_EditorCamera->transform.position = Vec3(0.0f, 0.0f, 0.0f);

    Scene* loaded = m_SceneManager.load(candidateContext.scene.string());
    if (!loaded) {
        Logger::error("Failed to load scene: " + candidateContext.scene.string());
        Path::setBase(oldBase.string());
        return false;
    }

    m_CurrentSceneInfo = candidateContext;
    m_ColliderDebugEntities.clear();
    m_SelectedEntity = nullptr;
    AddRecentScene(candidateContext.scene);
    return true;
}

// User Preferences
void Layer::LoadUserPreferences() {
    m_RecentScenes.clear();

    if (!fs::exists(m_UserPreferencesPath)) {
        SaveUserPreferences();
        return;
    }

    std::ifstream in(m_UserPreferencesPath);
    if (!in.is_open()) {
        Logger::warn("Could not open user preferences file for reading.");
        return;
    }

    try {
        nlohmann::json data;
        in >> data;

        if (data.contains("camera_sensitivity") && data["camera_sensitivity"].is_number()) {
            m_CameraSens = data["camera_sensitivity"].get<float>();
        }
        if (data.contains("camera_speed") && data["camera_speed"].is_number()) {
            m_CameraSpeed = data["camera_speed"].get<float>();
        }
        if (data.contains("window_width") && data.contains("window_height") &&
            data["window_width"].is_number() && data["window_height"].is_number()) {
            m_WindowSize = Vec2(
                data["window_width"].get<float>(),
                data["window_height"].get<float>()
            );
        }
        if (data.contains("style") && data["style"].is_string()) {
            std::string style = data["style"].get<std::string>();
            if (style == "Dark") {
                Styles::setupDarkTheme();
                m_ThemeStyle = "Dark";
            } else if (style == "Light") {
                ImGui::StyleColorsLight();
                m_ThemeStyle = "Light";
            }
        }

        if (data.contains("audio_enabled") && data["audio_enabled"].is_boolean()) {
            m_AudioEnabled = data["audio_enabled"].get<bool>();
        }
        if (data.contains("master_volume") && data["master_volume"].is_number()) {
            m_MasterVolume = std::clamp(data["master_volume"].get<float>(), 0.0f, 8.0f);
        }

        if (data.contains("recent_scenes") && data["recent_scenes"].is_array()) {
            for (const auto& scene : data["recent_scenes"]) {
                if (!scene.is_string()) {
                    continue;
                }

                fs::path scenePath(scene.get<std::string>());
                m_RecentScenes.push_back(scenePath.generic_string());
                if (m_RecentScenes.size() >= kMaxRecentScenes) {
                    break;
                }
            }
        }
    } catch (const std::exception& ex) {
        Logger::warn("Invalid user preferences JSON, using defaults: " + std::string(ex.what()));
    }
}

void Layer::SaveUserPreferences() const {
    try {
        fs::create_directories(m_UserPreferencesPath.parent_path());
    } catch (const std::exception& ex) {
        Logger::error("Failed to create preferences directory: " + std::string(ex.what()));
        return;
    }

    const Vec2 windowSize = m_Window.isFullscreen() ? m_WindowSize : m_Window.getSize();

    nlohmann::json data = {
        { "camera_sensitivity", m_CameraSens },
        { "camera_speed", m_CameraSpeed },
        { "style", m_ThemeStyle },
        { "audio_enabled", m_AudioEnabled },
        { "master_volume", m_MasterVolume },
        { "window_width", windowSize.x },
        { "window_height", windowSize.y },
        { "recent_scenes", m_RecentScenes }
    };

    std::ofstream out(m_UserPreferencesPath);
    if (!out.is_open()) {
        Logger::error("Could not open user preferences file for writing.");
        return;
    }

    out << data.dump(4);
}

void Layer::ClearUserPreferences() {
    m_CameraSens = 0.15f;
    m_CameraSpeed = 1.0f;
    m_AudioEnabled = true;
    m_MasterVolume = 1.0f;
    m_RecentScenes.clear();

    std::error_code ec;
    fs::remove(m_UserPreferencesPath, ec);
    if (ec) {
        Logger::warn("Failed to remove user preferences file: " + ec.message());
    }

    ApplyAudioSettings();
    SaveUserPreferences();
}

void Layer::AddRecentScene(const fs::path& scenePath) {
    const std::string normalizedPath = fs::absolute(scenePath).lexically_normal().generic_string();
    m_RecentScenes.erase(std::remove(m_RecentScenes.begin(), m_RecentScenes.end(), normalizedPath), m_RecentScenes.end());
    m_RecentScenes.insert(m_RecentScenes.begin(), normalizedPath);

    if (m_RecentScenes.size() > kMaxRecentScenes) {
        m_RecentScenes.resize(kMaxRecentScenes);
    }

    SaveUserPreferences();
}

void Layer::DrawDockspace() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    float menuBarHeight = ImGui::GetFrameHeight();
    const float statusBarHeight =
        (m_StatusPanel && m_StatusPanel->IsVisible())
            ? (ImGui::GetFrameHeightWithSpacing() - 2.5f)
            : 0.0f;

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - menuBarHeight - statusBarHeight));
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

    size_t pos = pathStr.find("/assets/"); // Windows problems?
    if (pos == std::string::npos) {
        Logger::error(std::format("Invalid scene path: '{}'. Must be inside the 'assets' directory.", inputPath)); 
        return { {}, {} };
    }

    return { fs::path(pathStr.substr(0, pos + 1)), fullPath };
}