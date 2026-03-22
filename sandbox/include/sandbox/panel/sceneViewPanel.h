#pragma once

#include "sandbox/panel/panel.h"

#include "engine/engine.h"
#include "engine/components/cameraComponent.h"
#include <imgui.h>

class SceneViewPanel : public Panel {
public:
    SceneViewPanel(Engine& engine, Entity*& editorCameraRef) 
        : m_Engine(engine), m_EditorCamera(editorCameraRef) {}

    void OnUIRender() override {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImVec2 available = ImGui::GetContentRegionAvail();

        if (available.x > 0 && available.y > 0 && (available.x != m_ViewportSize.x || available.y != m_ViewportSize.y)) {
            m_ViewportSize = available;
            m_Engine.getRenderer().resizeRenderTarget((unsigned int)available.x, (unsigned int)available.y);
            
            float aspectRatio = available.x / available.y;
            m_EditorCamera->getComponent<CameraComponent>()->getCamera().setAspectRatio(aspectRatio);
        }

        ImGui::Image((ImTextureID)(intptr_t)m_Engine.getRenderer().getRenderTexture(), available, ImVec2(0, 1), ImVec2(1, 0));

        static bool cameraLookActive = false;
        if (!cameraLookActive && ImGui::IsWindowHovered() && m_Engine.getInput().isMouseButtonPressed(MOUSE_RIGHT)) {
            cameraLookActive = true;
        }
        if (cameraLookActive && !m_Engine.getInput().isMouseButtonPressed(MOUSE_RIGHT)) {
            cameraLookActive = false;
        }

        m_Engine.getInput().setCursorMode(cameraLookActive);

        if (cameraLookActive) {
            HandleCameraInput();
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

private:
    void HandleCameraInput() {
        if (!m_EditorCamera) return;

        float speed = 200.0f * m_Engine.getDeltaTime();
        Input& input = m_Engine.getInput();
        if (input.isKeyPressed(KEY_SHIFT)) speed *= 2.0f;
        if (input.isKeyPressed(KEY_CTRL)) speed *= 0.5f;

        if (input.isKeyPressed(KEY_W)) m_EditorCamera->transform.position += m_EditorCamera->transform.forward() * speed;
        if (input.isKeyPressed(KEY_S)) m_EditorCamera->transform.position -= m_EditorCamera->transform.forward() * speed;
        if (input.isKeyPressed(KEY_A)) m_EditorCamera->transform.position += m_EditorCamera->transform.right() * speed;
        if (input.isKeyPressed(KEY_D)) m_EditorCamera->transform.position -= m_EditorCamera->transform.right() * speed;

        float sensitivity = 0.15f;
        Vec2 delta = input.getMouseDelta();
        m_EditorCamera->transform.rotation.y += delta.x * sensitivity;
        m_EditorCamera->transform.rotation.x -= delta.y * sensitivity;
    }

    Engine& m_Engine;
    Entity*& m_EditorCamera;
    ImVec2 m_ViewportSize = { 0, 0 };
};