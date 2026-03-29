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
#include "engine/utils/path.h"
#include "engine/utils/logger.h"

#include "sandbox/styles.h"

#include <iostream>

template <typename Setter>
static void DrawFilePicker(
    const char* label,
    const char* id,
    const std::string& currentPath,
    const std::vector<std::string>& filters,
    Setter setPath,
    bool showClearButton = true
) {
    ImGui::TextUnformatted(label);
    ImGui::SameLine();

    const float clearButtonWidth = ImGui::GetFrameHeight();
    const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
    float chooseButtonWidth = ImGui::GetContentRegionAvail().x - clearButtonWidth - spacing;
    if (chooseButtonWidth < 80.0f) {
        chooseButtonWidth = 80.0f;
    }

    const std::string chooseText = currentPath.empty() ? "Choose file..." : currentPath;
    const std::string chooseLabel = chooseText + "##choose_" + id;
    if (ImGui::Button(chooseLabel.c_str(), ImVec2(chooseButtonWidth, 0.0f))) {
        std::string chosenPath = Dialog::openFile(filters);
        if (!chosenPath.empty()) {
            setPath(chosenPath);
        }
    }

    if (!showClearButton) return;

    ImGui::SameLine(0.0f, spacing);
    if (currentPath.empty()) {
        ImGui::BeginDisabled();
    }
    const std::string clearLabel = std::string("X##clear_") + id;
    if (ImGui::Button(clearLabel.c_str(), ImVec2(clearButtonWidth, 0.0f))) {
        setPath("");
    }
    if (currentPath.empty()) {
        ImGui::EndDisabled();
    }
}

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
        if (ImGui::BeginListBox("Renderer", ImVec2(-1.0f, 300.0f))) {
            DrawFilePicker(
                "Model",
                "model_path",
                c->getModelPath(),
                { "Model Files", "*.obj *.fbx *.dae *.gltf *.glb", "All Files", "*" },
                [&](const std::string& path) { c->setModelPath(path); }
            );

            ImGui::SeparatorText("Shader");
            DrawFilePicker(
                "Vertex Shader",
                "vert_path",
                c->getVertPath(),
                { "Shader Files", "*.glsl *.vert *.vs", "All Files", "*" },
                [&](const std::string& path) { c->setVertPath(path); }
            );

            DrawFilePicker(
                "Fragment Shader",
                "frag_path",
                c->getFragPath(),
                { "Shader Files", "*.glsl *.frag *.fs", "All Files", "*" },
                [&](const std::string& path) { c->setFragPath(path); }
            );

            ImGui::SeparatorText("Textures");
            DrawFilePicker(
                "Diffuse Texture",
                "diffuse_path",
                c->getDiffuseTexturePath(),
                { "Image Files", "*.png *.jpg *.jpeg *.bmp *.tga *.hdr", "All Files", "*" },
                [&](const std::string& path) { c->setDiffuseTexturePath(path); }
            );

            DrawFilePicker(
                "Specular Texture",
                "specular_path",
                c->getSpecularTexturePath(),
                { "Image Files", "*.png *.jpg *.jpeg *.bmp *.tga *.hdr", "All Files", "*" },
                [&](const std::string& path) { c->setSpecularTexturePath(path); }
            );

            DrawFilePicker(
                "Normal Texture",
                "normal_path",
                c->getNormalTexturePath(),
                { "Image Files", "*.png *.jpg *.jpeg *.bmp *.tga *.hdr", "All Files", "*" },
                [&](const std::string& path) { c->setNormalTexturePath(path); }
            );

            ImGui::SeparatorText("Material");
            Vec3 baseColor = c->getBaseColor();
            if (ImGui::ColorEdit3("Base Color", &baseColor.x)) {
                c->setBaseColor(baseColor);
            }

            float ambientStrength = c->getAmbientStrength();
            if (ImGui::DragFloat("Ambient Strength", &ambientStrength, 0.01f, 0.0f, 2.0f)) {
                c->setAmbientStrength(ambientStrength);
            }

            float specularStrength = c->getSpecularStrength();
            if (ImGui::DragFloat("Specular Strength", &specularStrength, 0.05f, 0.0f, 8.0f)) {
                c->setSpecularStrength(specularStrength);
            }

            float shininess = c->getShininess();
            if (ImGui::DragFloat("Shininess", &shininess, 1.0f, 1.0f, 256.0f)) {
                c->setShininess(shininess);
            }

            Vec2 uvScale = c->getUVScale();
            float uv[2] = { uvScale.x, uvScale.y };
            if (ImGui::DragFloat2("UV Scale", uv, 0.1f, 0.01f, 128.0f)) {
                c->setUVScale(Vec2(uv[0], uv[1]));
            }

            ImGui::EndListBox();
        }
    });

    InspectorRegistry::registerComponent<SkyboxComponent>([](SkyboxComponent* c) {
        const std::vector<std::string> faces = c->getFaces();
        if (ImGui::BeginListBox("Faces", ImVec2(-1.0f, 0.0f))) {
            for (int i = 0; i < (int)faces.size(); i++) {
                ImGui::PushID(i);
                const std::string faceLabel = "Face " + std::to_string(i);
                DrawFilePicker(
                    faceLabel.c_str(),
                    "face_path",
                    faces[i],
                    { "Image Files", "*.png *.jpg *.jpeg *.bmp *.tga *.hdr", "All Files", "*" },
                    [&](const std::string& path) { c->setFacePath((size_t)i, path); },
                    false // no clear button
                );
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