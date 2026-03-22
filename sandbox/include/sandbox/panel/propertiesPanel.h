#pragma once

#include "sandbox/panel/panel.h"

#include "sandbox/inspectorRegistry.h"
#include "engine/engine.h"
#include <imgui.h>

class PropertiesPanel : public Panel {
public:
    PropertiesPanel(Engine& engine, Entity*& selectedRef) 
        : m_Engine(engine), m_SelectedEntity(selectedRef) {}

    void OnUIRender() override {
        ImGui::Begin("Properties");
        if (m_SelectedEntity) {
            char nameBuf[256];
            strncpy(nameBuf, m_SelectedEntity->name.c_str(), sizeof(nameBuf));
            if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf)))
                m_SelectedEntity->name = nameBuf;
                
            ImGui::SameLine();
            if (ImGui::Button("Delete Entity")) {
                m_Engine.getSceneManager().getActiveScene()->destroyEntity(m_SelectedEntity);
                m_SelectedEntity = nullptr;
                ImGui::End();
                return;
            }
            
            ImGui::Separator();

            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                float pos[3] = { m_SelectedEntity->transform.position.x, m_SelectedEntity->transform.position.y, m_SelectedEntity->transform.position.z };
                if (ImGui::DragFloat3("Position", pos, 0.5f)) m_SelectedEntity->transform.position = Vec3(pos[0], pos[1], pos[2]);

                float rot[3] = { m_SelectedEntity->transform.rotation.x, m_SelectedEntity->transform.rotation.y, m_SelectedEntity->transform.rotation.z };
                if (ImGui::DragFloat3("Rotation", rot, 0.5f)) m_SelectedEntity->transform.rotation = Vec3(rot[0], rot[1], rot[2]);

                float scale[3] = { m_SelectedEntity->transform.scale.x, m_SelectedEntity->transform.scale.y, m_SelectedEntity->transform.scale.z };
                if (ImGui::DragFloat3("Scale", scale, 0.01f)) m_SelectedEntity->transform.scale = Vec3(scale[0], scale[1], scale[2]);
            }

            for (const auto& [type, comp] : m_SelectedEntity->getComponents()) {
                if (ImGui::CollapsingHeader(type.name(), ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Checkbox("Enabled", &comp->isEnabled);
                    ImGui::SameLine();
                    if (ImGui::Button(("Remove Component"))) {
                        //m_SelectedEntity->removeComponent(type);
                        break;
                    }

                    ImGui::PushID(type.hash_code());
                    InspectorRegistry::draw(type, comp.get());
                    ImGui::PopID();
                }
            }
            
            ImGui::Separator();
            if (ImGui::Button("Add Component", ImVec2(-1, 0))) { /* Implementation placeholder */ }
        } else {
            ImGui::TextDisabled("No entity selected");
        }
        ImGui::End();
    }

private:
    Engine& m_Engine;
    Entity*& m_SelectedEntity;
};