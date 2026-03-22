#include "sandbox/layer.h"
#include "sandbox/inspectorRegistry.h"
#include "sandbox/panel/hierarchyPanel.h"
#include "sandbox/panel/propertiesPanel.h"
#include "sandbox/panel/sceneViewPanel.h"

#include "engine/components/cameraComponent.h"
#include "engine/components/rendererComponent.h"
#include "engine/components/skyboxComponent.h"
#include "engine/components/directionalLightComponent.h"
#include "engine/components/pointLightComponent.h"

#include "engine/input/input.h"
#include "engine/debug/path.h"
#include "engine/debug/logger.h"

#include "sandbox/styles.h"

#include <iostream>

void registerDefaultInspectors() {
    InspectorRegistry::registerComponent<CameraComponent>([](CameraComponent* c) {
        float fov = c->getCamera().getFov();
        if (ImGui::DragFloat("FOV", &fov, 0.5f, 10.0f, 170.0f))
            c->getCamera().setFov(fov);
    });

    InspectorRegistry::registerComponent<RenderComponent>([](RenderComponent* c) {
        ImGui::LabelText("Model", "%s", c->getModelPath().c_str());
        ImGui::LabelText("Vert",  "%s", c->getVertPath().c_str());
        ImGui::LabelText("Frag",  "%s", c->getFragPath().c_str());
    });

    InspectorRegistry::registerComponent<SkyboxComponent>([](SkyboxComponent* c) {
        for (int i = 0; i < (int)c->getFaces().size(); i++)
            ImGui::LabelText(("Face " + std::to_string(i)).c_str(), "%s", c->getFaces()[i].c_str());
    });

    InspectorRegistry::registerComponent<PointLightComponent>([](PointLightComponent* c) {
        Vec3 color = c->getColor();
        if (ImGui::ColorEdit3("Color", &color.x)) {
            c->setColor(color);
        }

        float intensity = c->getIntensity();
        if (ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 100.0f)) {
            c->setIntensity(intensity);
        }

        float radius = c->getRadius();
        if (ImGui::DragFloat("Radius", &radius, 1.0f, 0.0f, 1000.0f)) {
            c->setRadius(radius);
        }
    });

    InspectorRegistry::registerComponent<DirectionalLightComponent>([](DirectionalLightComponent* c) {
        Vec3 color = c->getColor();
        if (ImGui::ColorEdit3("Color", &color.x)) {
            c->setColor(color);
        }

        float intensity = c->getIntensity();
        if (ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 100.0f)) {
            c->setIntensity(intensity);
        }
    });
}

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

    // ----
    std::string input;
    std::cout << "[PLACEHOLDER] Enter scene file path: ";
    std::getline(std::cin, input);
    // ----

    registerDefaultInspectors();

    ScenePathInfo info = GetSceneContext(input);
    Path::setBase(info.root);
    
    m_SceneManager.load(info.scene.string());

    const std::string title = m_SceneManager.getActiveScene() 
    ? "Origin Sandbox - " + m_SceneManager.getActiveScene()->name 
    : "Origin Sandbox";

    m_Window.setWindowTitle(title.c_str());

    Entity* cam = m_Engine.createEntity("Editor Camera");
    cam->addComponent<CameraComponent>(60.0f, m_Window.getAspectRatio(), 0.1f, 10000.0f);
    cam->transform.position = Vec3(0.0f, 150.0f, 500.0f);
    m_EditorCamera = cam;

    m_Panels.push_back(std::make_unique<HierarchyPanel>(m_Engine, m_SelectedEntity));
    m_Panels.push_back(std::make_unique<PropertiesPanel>(m_Engine, m_SelectedEntity));
    m_Panels.push_back(std::make_unique<SceneViewPanel>(m_Engine, m_EditorCamera));
}

void Layer::OnUIRender() {
    DrawMenuBar();
    DrawDockspace();
    
    // DEBUG!! (styling)
    //ImGui::ShowStyleEditor();

    for (auto& panel : m_Panels) {
        panel->OnUIRender();
    }
}

void Layer::DrawMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) m_Engine.getSceneManager().createScene("Empty Scene");
            ImGui::Separator();
            if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {}
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {}
            if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Quit")) m_Engine.stop();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Entity")) {
            if (ImGui::MenuItem("Create Empty")) {
                Entity* entity = m_Engine.createEntity("Entity");
                m_Engine.moveToScene(entity);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("Set Fullscreen", "F11")) {
                m_Window.setFullscreen(!m_Window.isFullscreen());
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Lighting")) {
            if (ImGui::MenuItem("Toggle Lighting", "Ctrl+L")) {
                m_Renderer.setLightingEnabled(!m_Renderer.isLightingEnabled());
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                if (!m_AboutPanel) {
                    m_AboutPanel = new AboutPanel();
                    m_Panels.push_back(std::unique_ptr<Panel>(m_AboutPanel));
                }
                m_AboutPanel->Open();
            }
            if (ImGui::MenuItem("GitHub")) {
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
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
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