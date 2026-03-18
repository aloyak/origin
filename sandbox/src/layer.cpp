#include "sandbox/layer.h"
#include "sandbox/inspectorRegistry.h"

#include "engine/components/camera.h"
#include "engine/components/renderer.h"
#include "engine/components/skybox.h"

#include "engine/input/input.h"
#include "engine/components/camera.h"
#include "engine/debug/path.h"

#include "engine/debug/logger.h"

#include <iostream>

Layer::Layer(Engine& engine) 
    : m_Engine(engine) {
    
    ImGuiIO& io = m_Engine.getIO();
    static std::string layout = Path::resolve("resources/layout.ini").string();
    io.IniFilename = layout.c_str();

    std::string input;
    std::cout << "Enter scene file path: "; // Placeholder!
    std::getline(std::cin, input);

    
    // Register Components
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


    ScenePathInfo info = GetSceneContext(input);
    Path::setBase(info.root);

    SceneManager& sm = m_Engine.getSceneManager();
    sm.load(info.scene.string());

    Entity* cam = m_Engine.createEntity("Editor Camera");
    cam->addComponent<CameraComponent>(60.0f, m_Window.getAspectRatio(), 0.1f, 10000.0f);
    cam->transform.position = Vec3(0.0f, 150.0f, 500.0f);
    m_EditorCamera = cam;
}

void Layer::OnUIRender() {
    DrawMenuBar();
    DrawDockspace();
    DrawSceneView();
    DrawHierarchy();
    DrawProperties();
}

void Layer::DrawMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) m_Engine.getSceneManager().createScene("Empty Scene");
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) m_Engine.stop();
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
        if (ImGui::BeginMenu("Help")) {
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

void Layer::DrawSceneView() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 available = ImGui::GetContentRegionAvail();

    if (available.x > 0 && available.y > 0 && (available.x != m_ViewportSize.x || available.y != m_ViewportSize.y)) {
        m_ViewportSize = available;
        
        m_Engine.resizeRenderTarget((unsigned int)available.x, (unsigned int)available.y);
        
        float aspectRatio = available.x / available.y;
        m_EditorCamera->getComponent<CameraComponent>()->getCamera().setAspectRatio(aspectRatio);
    }

    ImGui::Image((ImTextureID)(intptr_t)m_Engine.getRenderTexture(), available, ImVec2(0, 1), ImVec2(1, 0));

    static bool cameraLookActive = false;
    if (!cameraLookActive && ImGui::IsWindowHovered() && m_Input.isMouseButtonPressed(MOUSE_RIGHT)) {
        cameraLookActive = true;
    }
    if (cameraLookActive && !m_Input.isMouseButtonPressed(MOUSE_RIGHT)) {
        cameraLookActive = false;
    }

    m_Input.setCursorMode(cameraLookActive);

    if (cameraLookActive) {
        HandleCameraInput();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void Layer::HandleCameraInput() {
    if (!m_EditorCamera) return;

    float speed = 200.0f * m_Engine.getDeltaTime();
    if (m_Input.isKeyPressed(KEY_SHIFT)) speed *= 2.0f;
    if (m_Input.isKeyPressed(KEY_CTRL)) speed *= 0.5f;

    if (m_Input.isKeyPressed(KEY_W)) m_EditorCamera->transform.position += m_EditorCamera->transform.forward() * speed;
    if (m_Input.isKeyPressed(KEY_S)) m_EditorCamera->transform.position -= m_EditorCamera->transform.forward() * speed;
    if (m_Input.isKeyPressed(KEY_A)) m_EditorCamera->transform.position += m_EditorCamera->transform.right() * speed;
    if (m_Input.isKeyPressed(KEY_D)) m_EditorCamera->transform.position -= m_EditorCamera->transform.right() * speed;

    float sensitivity = 0.15f;
    Vec2 delta = m_Input.getMouseDelta();
    m_EditorCamera->transform.rotation.y += delta.x * sensitivity;
    m_EditorCamera->transform.rotation.x -= delta.y * sensitivity;
}

void Layer::DrawHierarchy() {
    ImGui::Begin("Hierarchy");

    auto* scene = m_Engine.getSceneManager().getActiveScene();
    const char* label = scene ? scene->name.c_str() : "No Scene";

    if (ImGui::Button("+")) {
        Entity* entity = m_Engine.createEntity("Entity");
        m_Engine.moveToScene(entity);
    }

    if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen)) {
        if (scene) {
            for (auto& entity : scene->getEntities()) {
                bool selected = (m_SelectedEntity == entity.get());
                if (ImGui::Selectable(entity->name.c_str(), selected))
                    m_SelectedEntity = entity.get();
            }
        }
        ImGui::TreePop();
    }

    ImGui::End();
}

void Layer::DrawProperties() {
    ImGui::Begin("Properties");
    if (m_SelectedEntity) {
        char nameBuf[256];
        strncpy(nameBuf, m_SelectedEntity->name.c_str(), sizeof(nameBuf));
        if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf)))
            m_SelectedEntity->name = nameBuf;
            
        ImGui::Separator();

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            float pos[3] = { m_SelectedEntity->transform.position.x, m_SelectedEntity->transform.position.y, m_SelectedEntity->transform.position.z };
            if (ImGui::DragFloat3("Position", pos, 0.5f)) m_SelectedEntity->transform.position = Vec3(pos[0], pos[1], pos[2]);

            float rot[3] = { m_SelectedEntity->transform.rotation.x, m_SelectedEntity->transform.rotation.y, m_SelectedEntity->transform.rotation.z };
            if (ImGui::DragFloat3("Rotation", rot, 0.5f)) m_SelectedEntity->transform.rotation = Vec3(rot[0], rot[1], rot[2]);

            float scale[3] = { m_SelectedEntity->transform.scale.x, m_SelectedEntity->transform.scale.y, m_SelectedEntity->transform.scale.z };
            if (ImGui::DragFloat3("Scale", scale, 0.01f)) m_SelectedEntity->transform.scale = Vec3(scale[0], scale[1], scale[2]);
        }
        ImGui::Separator();

        for (const auto& [type, comp] : m_SelectedEntity->getComponents()) {
            if (ImGui::CollapsingHeader(type.name(), ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("Enabled", &comp->isEnabled);
                ImGui::SameLine();
                if (ImGui::Button(("Remove Component"))) {
                    m_SelectedEntity->removeComponent<std::remove_pointer_t<decltype(comp.get())>>();
                    break;
                }

                ImGui::PushID(type.hash_code());
                InspectorRegistry::draw(type, comp.get());
                ImGui::PopID();
            }
            ImGui::Separator();
        }

        if (ImGui::Button("Add Component")) {
            //
        }
    } else {
        ImGui::TextDisabled("No entity selected");
    }
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