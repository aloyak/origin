#include "sandbox/layer.h"
#include "sandbox/inspectorRegistry.h"
#include "sandbox/panel/hierarchyPanel.h"
#include "sandbox/panel/propertiesPanel.h"
#include "sandbox/panel/sceneViewPanel.h"
#include "sandbox/dialog.h"

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
        float nearFar[2] = { c->getCamera().getNear(), c->getCamera().getFar() };
        if (ImGui::DragFloat2("Near/Far", nearFar, 0.1f, 0.01f, 10000.0f, "Near: %.2f\nFar: %.2f")) {
            if (nearFar[1] <= nearFar[0]) {
                nearFar[1] = nearFar[0] + 0.01f;
            }
            c->getCamera().setNear(nearFar[0]);
            c->getCamera().setFar(nearFar[1]);
        }
    });

    InspectorRegistry::registerComponent<RenderComponent>([](RenderComponent* c) {
        ImGui::LabelText("Model", "%s", c->getModelPath().c_str());
        ImGui::LabelText("Vert",  "%s", c->getVertPath().c_str());
        ImGui::LabelText("Frag",  "%s", c->getFragPath().c_str());
    });

    InspectorRegistry::registerComponent<SkyboxComponent>([](SkyboxComponent* c) {
        const std::vector<std::string> faces = c->getFaces();
        if (ImGui::BeginListBox("Faces")) {
            for (int i = 0; i < (int)faces.size(); i++) {
                ImGui::PushID(i);
                ImGui::Text("Face %d", i);
                ImGui::SameLine();

                const std::string buttonLabel = faces[i].empty() ? "Choose file...##face_path" : faces[i] + "##face_path";
                if (ImGui::Button(buttonLabel.c_str())) {
                    std::string chosenPath = Dialog::openFile({ "Image Files", "*.png *.jpg *.jpeg *.bmp *.tga *.hdr", "All Files", "*" });
                    if (!chosenPath.empty()) {
                        c->setFacePath((size_t)i, chosenPath);
                    }
                }
                ImGui::PopID();
            }
            ImGui::EndListBox();
        }
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
        Vec3 direction = c->getDirection();
        if (ImGui::DragFloat3("Direction", &direction.x, 0.1f, -1.0f, 1.0f)) {
            c->setDirection(direction.normalize());
        }

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

    registerDefaultInspectors();

    Entity* cam = m_Engine.createEntity("Editor Camera");
    cam->addComponent<CameraComponent>(60.0f, m_Window.getAspectRatio(), 0.1f, 10000.0f);
    cam->transform.position = Vec3(0.0f, 150.0f, 0.0f);
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

    const std::string title = m_SceneManager.getActiveScene() 
    ? "Origin Sandbox - " + m_SceneManager.getActiveScene()->name 
    : "Origin Sandbox";
    m_Window.setWindowTitle(title.c_str());
}

void Layer::OpenScene() {
    std::string path = Dialog::openFile({ "Scene Files", "*.json" });
    if (!path.empty()) {
        m_CurrentSceneInfo = GetSceneContext(path);
        Path::setBase(m_CurrentSceneInfo.root);
        if (m_CurrentSceneInfo.scene.empty()) {
            Logger::error("Failed to load scene: Invalid path.");
        } else {
            m_Engine.getSceneManager().load(m_CurrentSceneInfo.scene.string());
        }
    }
}

void Layer::SaveScene() {
    if (!m_SceneManager.getActiveScene()) {
        Logger::error("No active scene to save.");
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
}

void Layer::DrawMenuBar() {
    HandleShortcuts();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) m_Engine.getSceneManager().createScene("Empty Scene");
            ImGui::Separator();

            if (ImGui::MenuItem("Open Scene", "Ctrl+O")) OpenScene();
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) SaveScene();
            if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S")) SaveSceneAs();
            if (ImGui::MenuItem("Unload Scene")) UnloadScene();

            ImGui::Separator();
            if (ImGui::MenuItem("Quit")) m_Engine.stop();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("Set Fullscreen", "F11")) {
                m_Window.setFullscreen(!m_Window.isFullscreen());
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Rendering")) {
            bool isPixelArt = m_Renderer.isPixelArtEnabled();
            if (ImGui::MenuItem("Toggle Pixelart", "", &isPixelArt)) {
                if (isPixelArt) {
                    m_Renderer.setupRenderTarget(350, 200);
                    m_Renderer.setPixelArt(true, 4); // not working?
                } else {
                    Vec2 size = m_Window.getSize();
                    m_Renderer.setupRenderTarget((unsigned int)size.x, (unsigned int)size.y);
                    m_Renderer.setPixelArt(false, 32);
                }
            }
            if (ImGui::MenuItem("Toggle Vertex Snap", "", m_Renderer.isVertexSnapEnabled())) {
                m_Renderer.setVertexSnap(!m_Renderer.isVertexSnapEnabled());
            }
            ImGui::Separator();
            bool isLighting = m_Renderer.isLightingEnabled();
            if (ImGui::MenuItem("Toggle Lighting", "Ctrl+L", &isLighting)) {
                m_Renderer.setLightingEnabled(isLighting);
            }
            float ambient = m_Renderer.getMinimumAmbientLight();
            ImGui::TextUnformatted("Ambient Light");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(70.0f);
            if (ImGui::DragFloat("##AmbientLight", &ambient, 0.01f, 0.0f)) {
                m_Renderer.setMinimumAmbientLight(ambient);
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

void Layer::HandleShortcuts() {
    static int cooldown = 0;
    if (cooldown > 0) {
        cooldown--;
        return;
    }

    if (m_Input.isKeyPressed(KEY_F11)) {
        m_Window.setFullscreen(!m_Window.isFullscreen());
        cooldown = 30;
    }
    if (m_Input.isKeyDown(KEY_LCTRL) && m_Input.isKeyPressed(KEY_N)) {
        m_Engine.getSceneManager().createScene("Empty Scene");
        cooldown = 30;
    }
    if (m_Input.isKeyDown(KEY_LCTRL) && m_Input.isKeyPressed(KEY_O)) {
        OpenScene();
        cooldown = 30;
    }
    if (m_Input.isKeyDown(KEY_LCTRL) && m_Input.isKeyPressed(KEY_S)) {
        SaveScene();
        cooldown = 30;
    }
    if (m_Input.isKeyDown(KEY_LCTRL) && m_Input.isKeyDown(KEY_LSHIFT) && m_Input.isKeyPressed(KEY_S)) {
        SaveSceneAs();
        cooldown = 30;
    }

    if (m_Input.isKeyDown(KEY_LCTRL) && m_Input.isKeyPressed(KEY_L)) {
        m_Renderer.setLightingEnabled(!m_Renderer.isLightingEnabled());
        cooldown = 30;
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