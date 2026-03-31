#pragma once

#include "sandbox/panel/panel.h"
#include "engine/engine.h"
#include <imgui.h>

class HierarchyPanel : public Panel {
public:
    HierarchyPanel(Engine& engine, Entity*& selectedRef) 
        : m_Engine(engine), m_SelectedEntity(selectedRef) {}

    void OnUIRender() override {
        ImGui::Begin("Hierarchy");
        auto* scene = m_Engine.getSceneManager().getActiveScene();
        const char* label = scene ? scene->name.c_str() : "No Scene";

        if (ImGui::Button("Create Entity")) {
            Entity* entity = m_Engine.createEntity("Entity");
            m_Engine.moveToScene(entity);
        }
        ImGui::SameLine();
        if (ImGui::Button("Duplicate Entity") && m_SelectedEntity) {
            Entity* newEntity = scene->createEntity(m_SelectedEntity->name + " Copy");
            newEntity->transform = m_SelectedEntity->transform;
            for (const auto& [typeId, component] : m_SelectedEntity->getComponents()) {
                newEntity->addComponentCopy(typeId, component);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete Entity") && m_SelectedEntity) {
            scene->destroyEntity(m_SelectedEntity);
            m_SelectedEntity = nullptr;
        }
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth)) {
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

private:
    Engine& m_Engine;
    Entity*& m_SelectedEntity;
};