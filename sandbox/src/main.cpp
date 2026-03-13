#include "engine/engine.h"
#include "engine/components/camera.h"
#include "engine/components/entity.h"

#include <imgui.h>

int main() {
    Engine engine(1600, 900, "Sandbox");
    engine.initUI();
    engine.enableVSync(true);

    engine.setupRenderTarget(1600, 900);

    ImGuiIO& io = engine.getIO();
    io.IniFilename = "sandbox/resources/layout.ini";    // TODO: fix relative paths

    // engine.setPixelArt(true, 8);

    SceneManager& sm = engine.getSceneManager();
    sm.load("assets/scenes/sponza.json");

    
    Entity* cam = engine.createEntity("Editor Camera");
    cam->addComponent<CameraComponent>(60.0f, engine.getAspectRatio(), 0.1f, 10000.0f);
    cam->transform.position = Vec3(0.0f, 150.0f, 500.0f);
    engine.moveToScene(cam);

    Entity* selectedEntity  = nullptr;
    ImVec2  viewportSize    = { 1600, 900 };

    engine.run([&]() {
        engine.beginUI();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("##dockspace", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize   | ImGuiWindowFlags_NoMove     |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoBackground);
        ImGui::PopStyleVar();
        ImGui::DockSpace(ImGui::GetID("MainDockspace"), ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImVec2 available = ImGui::GetContentRegionAvail();

        if (available.x != viewportSize.x || available.y != viewportSize.y) {
            viewportSize = available;
            engine.resizeRenderTarget(
                (unsigned int)available.x,
                (unsigned int)available.y
            );
        }

        // OpenGL textures are bottom-up
        ImGui::Image(
            (ImTextureID)(intptr_t)engine.getRenderTexture(),
            available,
            ImVec2(0, 1), ImVec2(1, 0)
        );
        ImGui::End();
        ImGui::PopStyleVar();

        // Hierarchy ---
        ImGui::Begin("Hierarchy");
        if (engine.getSceneManager().getActiveScene()) {
            for (auto& entity : engine.getSceneManager().getActiveScene()->getEntities()) {
                bool selected = (selectedEntity == entity.get());
                if (ImGui::Selectable(entity->name.c_str(), selected))
                    selectedEntity = entity.get();
            }
        }
        ImGui::End();

        // Properties ---
        ImGui::Begin("Properties");
        if (selectedEntity) {
            ImGui::Text("Name: %s", selectedEntity->name.c_str());
            ImGui::Separator();

            float pos[3] = {
                selectedEntity->transform.position.x,
                selectedEntity->transform.position.y,
                selectedEntity->transform.position.z
            };
            if (ImGui::DragFloat3("Position", pos, 0.5f)) {
                selectedEntity->transform.position = Vec3(pos[0], pos[1], pos[2]);
            }

            float rot[3] = {
                selectedEntity->transform.rotation.x,
                selectedEntity->transform.rotation.y,
                selectedEntity->transform.rotation.z
            };
            if (ImGui::DragFloat3("Rotation", rot, 0.5f)) {
                selectedEntity->transform.rotation = Vec3(rot[0], rot[1], rot[2]);
            }

            float scale[3] = {
                selectedEntity->transform.scale.x,
                selectedEntity->transform.scale.y,
                selectedEntity->transform.scale.z
            };
            if (ImGui::DragFloat3("Scale", scale, 0.01f)) {
                selectedEntity->transform.scale = Vec3(scale[0], scale[1], scale[2]);
            }
        } else {
            ImGui::TextDisabled("No entity selected");
        }
        ImGui::End();

        engine.endUI();
    });
}