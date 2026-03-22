#pragma once

#include "sandbox/panel/panel.h"
#include <imgui.h>

class AboutPanel : public Panel {
public:
    void OnUIRender() override {
        if (!m_IsOpen) return;

        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("About Origin Sandbox", &m_IsOpen, 
                ImGuiWindowFlags_NoCollapse | 
                ImGuiWindowFlags_NoResize | 
                ImGuiWindowFlags_AlwaysAutoResize | 
                ImGuiWindowFlags_NoDocking
        )) {
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]); // ExtraBold
            ImGui::Text("Origin Sandbox");
            ImGui::PopFont();

            ImGui::Separator();
            ImGui::Text("By 4loyak!");
            ImGui::Text("Sandbox Scene Editor for the Origin Engine");
            ImGui::Text("More info: https://github.com/aloyak/origin");
        }
        ImGui::End();
    }

    void Open() { m_IsOpen = true; }
    bool IsOpen() const { return m_IsOpen; }

private:
    bool m_IsOpen = false;
};