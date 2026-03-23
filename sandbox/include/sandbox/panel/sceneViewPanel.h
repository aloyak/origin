#pragma once

#include "sandbox/panel/panel.h"

#include "engine/engine.h"
#include "engine/components/cameraComponent.h"
#include <imgui.h>

#include <ImGuizmo.h>

class SceneViewPanel : public Panel {
public:
    SceneViewPanel(Engine& engine, Entity*& editorCameraRef, float& cameraSpeedRef, Entity*& selectedEntityRef, ImGuizmo::OPERATION& gizmoOperationRef, bool& showRenderStatsRef)
        : m_Engine(engine), m_EditorCamera(editorCameraRef), m_CameraSpeed(cameraSpeedRef), m_SelectedEntity(selectedEntityRef), m_GizmoOperation(gizmoOperationRef), m_ShowRenderStats(showRenderStatsRef) {}

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

        DrawRenderStatsOverlay();
        DrawGizmos();

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

    void DrawGizmos() {
        if (!m_EditorCamera) return;

        CameraComponent* cameraComp = m_EditorCamera->getComponent<CameraComponent>();
        if (!cameraComp) return;

        const Camera& camera = cameraComp->getCamera();
        const Transform& cameraTransform = m_EditorCamera->transform;

        ImVec2 viewportMin = ImGui::GetItemRectMin();
        ImVec2 viewportSize = ImGui::GetItemRectSize();
        if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f) return;

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::BeginFrame();
        ImGuizmo::SetDrawlist();

        ImGuizmo::SetRect(viewportMin.x, viewportMin.y, viewportSize.x, viewportSize.y);

        const float* view = static_cast<const float*>(camera.getViewMatrix(cameraTransform));
        const float* projection = static_cast<const float*>(camera.getProjectionMatrix());

        if (!m_SelectedEntity) return;

        float model[16];
        BuildTransformMatrix(m_SelectedEntity->transform, model);

        ImGuizmo::Manipulate(
            view,
            projection,
            m_GizmoOperation,
            ImGuizmo::MODE::WORLD,
            model
        );

        if (ImGuizmo::IsUsing()) {
            float translation[3];
            float rotation[3];
            float scale[3];
            ImGuizmo::DecomposeMatrixToComponents(model, translation, rotation, scale);

            m_SelectedEntity->transform.position = Vec3(translation[0], translation[1], translation[2]);
            m_SelectedEntity->transform.rotation = Vec3(rotation[0], rotation[1], rotation[2]);
            m_SelectedEntity->transform.scale = Vec3(scale[0], scale[1], scale[2]);
        }
    }

    void DrawRenderStatsOverlay() {
        if (!m_ShowRenderStats) return;

        ImVec2 viewportMin = ImGui::GetItemRectMin();
        ImVec2 viewportSize = ImGui::GetItemRectSize();
        if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f) return;

        const float padding = 12.0f;
        const ImVec2 textSizeFps = ImGui::CalcTextSize("FPS 0000");
        const ImVec2 textSizeMs = ImGui::CalcTextSize("000.00 ms");
        const float contentWidth = textSizeFps.x > textSizeMs.x ? textSizeFps.x : textSizeMs.x;
        const ImVec2 overlaySize(contentWidth + 20.0f, textSizeFps.y + textSizeMs.y + 16.0f);

        ImGui::SetNextWindowPos(
            ImVec2(viewportMin.x + viewportSize.x - padding, viewportMin.y + padding),
            ImGuiCond_Always,
            ImVec2(1.0f, 0.0f)
        );
        ImGui::SetNextWindowSize(overlaySize, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.65f);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 6.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);

        constexpr ImGuiWindowFlags overlayFlags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoInputs;

        if (ImGui::Begin("##SceneRenderStatsOverlay", nullptr, overlayFlags)) {
            const float rawDeltaTime = m_Engine.getDeltaTime();
            const float deltaTime = rawDeltaTime > 0.000001f ? rawDeltaTime : 0.000001f;

            if (m_SmoothedDeltaTime <= 0.0f) {
                m_SmoothedDeltaTime = deltaTime;
            } else {
                constexpr float smoothing = 0.1f;
                m_SmoothedDeltaTime += (deltaTime - m_SmoothedDeltaTime) * smoothing;
            }

            const float fps = 1.0f / m_SmoothedDeltaTime;
            const float frameMs = m_SmoothedDeltaTime * 1000.0f;

            m_StatsTimer += deltaTime;
            if (m_StatsTimer >= m_StatsUpdateInterval) {
                m_StatsTimer = 0.0f;
                m_DisplayedFps = fps;
                m_DisplayedFrameMs = frameMs;
            }

            ImGui::Text("FPS %4.0f", m_DisplayedFps);
            ImGui::Text("%6.2f ms", m_DisplayedFrameMs);
        }
        ImGui::End();
        ImGui::PopStyleVar(2);
    }

private:
    static void BuildTransformMatrix(const Transform& t, float* outMatrix) {
        float translation[3] = { t.position.x, t.position.y, t.position.z };
        float rotation[3] = { t.rotation.x, t.rotation.y, t.rotation.z };
        float scale[3] = { t.scale.x, t.scale.y, t.scale.z };
        ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, outMatrix);
    }

    void HandleCameraInput() {
        if (!m_EditorCamera) return;

        float speed = 200.0f * m_CameraSpeed * m_Engine.getDeltaTime();
        Input& input = m_Engine.getInput();
        if (input.isKeyPressed(KEY_LSHIFT)) speed *= 3.0f;
        if (input.isKeyPressed(KEY_LCTRL)) speed *= 0.5f;

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
    float& m_CameraSpeed;
    Entity*& m_SelectedEntity;
    ImGuizmo::OPERATION& m_GizmoOperation;
    bool& m_ShowRenderStats;
    ImVec2 m_ViewportSize = { 0, 0 };
    float m_SmoothedDeltaTime = 0.0f;
    float m_StatsTimer = 0.0f;
    float m_StatsUpdateInterval = 0.12f;
    float m_DisplayedFps = 0.0f;
    float m_DisplayedFrameMs = 0.0f;
};