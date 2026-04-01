#pragma once

#include "sandbox/panel/panel.h"
#include "engine/engine.h"
#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

class HierarchyPanel : public Panel {
public:
    HierarchyPanel(Engine& engine, Entity*& selectedRef) 
        : m_Engine(engine), m_SelectedEntity(selectedRef) {}

    void OnUIRender() override {
        ImGui::Begin("Hierarchy");
        auto* scene = m_Engine.getSceneManager().getActiveScene();
        const char* label = scene ? scene->name.c_str() : "No Active Scene";

        ImGui::BeginGroup();

        if (ImGui::Button("+")) {
            if (scene) {
                Entity* entity = scene->createEntity("Entity");
                m_SelectedEntity = entity;
            }
        }

        if (ImGui::IsItemHovered()) ImGui::OpenPopup("EntityOptions");

        if (ImGui::BeginPopup("EntityOptions")) {
            if (ImGui::MenuItem("Create Entity") && scene) {
                Entity* entity = scene->createEntity("Entity");
                m_SelectedEntity = entity;
            }
            if (ImGui::MenuItem("Duplicate Entity") && m_SelectedEntity && scene) {
                Entity* newEntity = scene->createEntity(m_SelectedEntity->name + " Copy");
                newEntity->transform = m_SelectedEntity->transform;
                for (const auto& [typeId, component] : m_SelectedEntity->getComponents()) {
                    newEntity->addComponentCopy(typeId, component);
                }
            }
            if (ImGui::MenuItem("Delete Entity") && m_SelectedEntity && scene) {
                scene->destroyEntity(m_SelectedEntity);
                m_SelectedEntity = nullptr;
            }
            ImGui::EndPopup();
        }

        ImGui::SameLine();

        static char searchBuffer[256] = "";
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputTextWithHint("##Search", "Search ...", searchBuffer, sizeof(searchBuffer));

            std::vector<Entity*> filteredEntities;
            if (scene) {
                const bool hasSearch = searchBuffer[0] != '\0';
                std::string searchTerm = searchBuffer;
                std::transform(searchTerm.begin(), searchTerm.end(), searchTerm.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                const auto& entities = scene->getEntities();
                filteredEntities.reserve(entities.size());

                for (const auto& entity : entities) {
                    if (!entity) continue;

                    if (!hasSearch) {
                        filteredEntities.push_back(entity.get());
                        continue;
                    }

                    std::string entityName = entity->name;
                    std::transform(entityName.begin(), entityName.end(), entityName.begin(),
                        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                    if (entityName.find(searchTerm) != std::string::npos) {
                        filteredEntities.push_back(entity.get());
                    }
                }
        }
        ImGui::EndGroup();

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        bool treeOpen = ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth);
        ImGui::PopFont();

        if (treeOpen) {
            for (Entity* entity : filteredEntities) {
                bool selected = (m_SelectedEntity == entity);
                if (ImGui::Selectable(entity->name.c_str(), selected))
                    m_SelectedEntity = entity;
            }
            ImGui::TreePop();
        }
        ImGui::End();
    }

private:
    Engine& m_Engine;
    Entity*& m_SelectedEntity;
};