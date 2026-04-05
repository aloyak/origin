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
            float textWidth = ImGui::CalcTextSize("Origin Sandbox").x;
            ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - textWidth) * 0.5f);
            ImGui::Text("Origin Sandbox");
            ImGui::PopFont();

            ImGui::SeparatorText("About");
            ImGui::Text("By 4loyak!");
            ImGui::Text("Sandbox Scene Editor for the Origin Engine");
            ImGui::Text("More info: ");
            ImGui::SameLine();
            std::string url = "https://github.com/aloyak/origin";
            if (ImGui::TextLink(url.c_str())) {
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

            ImGui::SeparatorText("Dependencies");
            ImGui::BulletText(" OpenGL — 4.1");
            ImGui::BulletText(" Bullet Physics — 3.25");
            ImGui::BulletText(" SDL2 — 2.32.8");
            ImGui::BulletText(" Assimp — 6.0.4");
        }
        ImGui::End();
    }

    void Open() { m_IsOpen = true; }
    bool IsOpen() const { return m_IsOpen; }

private:
    bool m_IsOpen = false;
};