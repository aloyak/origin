#pragma once

#include "sandbox/panel/panel.h"
#include <imgui.h>

#include <string>

class StatusPanel : public Panel {
public:
    void SetConsole(const std::string& text) { m_ConsoleText = text; }
    void SetVisible(bool visible) { m_IsVisible = visible; }
    void ToggleVisible() { m_IsVisible = !m_IsVisible; }
    bool IsVisible() const { return m_IsVisible; }

    void OnUIRender() override {
        if (!m_IsVisible) return;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        const float barHeight = ImGui::GetFrameHeightWithSpacing() - 2.5f;

        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - barHeight));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, barHeight));
        
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.03f, 0.03f, 0.03f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 2.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

        ImGui::Begin("##StatusBar", nullptr,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextUnformatted(m_ConsoleText.c_str());
        ImGui::End();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
    }

private:
    bool m_IsVisible = true;
    std::string m_ConsoleText = "Ready!";
};