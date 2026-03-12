#include "engine/scene/sceneManager.h"
#include "engine/components/renderer.h"
#include "engine/components/camera.h"
#include "spdlog/spdlog.h"

#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

Component* createComponentByType(Entity* entity, const std::string& type, const json& jComp) {
    if (type == "RenderComponent") {
        std::string path = jComp.value("model", ""); 
        return entity->addComponent<RenderComponent>(path);
    }
    if (type == "CameraComponent") {
        return entity->addComponent<CameraComponent>(60.0f, 1.0f, 0.1f, 100.0f);
    }
    return nullptr;
}

void SceneManager::unload() {
    if (m_activeScene) {
        delete m_activeScene;
        m_activeScene = nullptr;
        spdlog::info("Scene unloaded.");
    }
}

Entity* SceneManager::createEntity(std::string name) {
    if (!m_activeScene) {
        m_activeScene = new Scene();
        m_activeScene->name = "Default Scene";
    }
    return m_activeScene->createEntity(name);
}

void SceneManager::save(const std::string& scenePath) {
    if (!m_activeScene) {
        spdlog::warn("No active scene to save.");
        return;
    }

    json j;
    j["name"] = m_activeScene->name;
    j["entities"] = json::array();

    for (const auto& entity : m_activeScene->getEntities()) {
        json jEnt;
        jEnt["name"] = entity->name;
        jEnt["transform"] = {
            {"pos", {entity->transform.position.x, entity->transform.position.y, entity->transform.position.z}},
            {"rot", {entity->transform.rotation.x, entity->transform.rotation.y, entity->transform.rotation.z}},
            {"sca", {entity->transform.scale.x, entity->transform.scale.y, entity->transform.scale.z}}
        };

        jEnt["components"] = json::array();
        for (const auto& [typeIdx, comp] : entity->getComponents()) {
            json jComp;
            comp->serialize(jComp);
            if (!jComp.empty()) {
                jEnt["components"].push_back(jComp);
            }
        }
        j["entities"].push_back(jEnt);
    }

    std::ofstream file(scenePath);
    if (file.is_open()) {
        file << j.dump(4);
        spdlog::info("Scene saved to: {}", scenePath);
    } else {
        spdlog::error("Failed to open file for saving: {}", scenePath);
    }
}

Scene* SceneManager::load(const std::string& scenePath) {
    std::ifstream file(scenePath);
    if (!file.is_open()) {
        spdlog::error("Could not open scene file: {}", scenePath);
        return nullptr;
    }

    json j;
    try {
        file >> j;
    } catch (json::parse_error& e) {
        spdlog::error("JSON parse error in {}: {}", scenePath, e.what());
        return nullptr;
    }

    unload();
    m_activeScene = new Scene();
    m_activeScene->name = j.value("name", "New Scene");

    if (j.contains("entities") && j["entities"].is_array()) {
        for (const auto& jEnt : j["entities"]) {
            Entity* ent = m_activeScene->createEntity(jEnt.value("name", "Entity"));

            if (jEnt.contains("transform")) {
                const auto& t = jEnt["transform"];
                if (t.contains("pos") && t["pos"].size() >= 3) {
                    ent->transform.position = { t["pos"][0], t["pos"][1], t["pos"][2] };
                }
                if (t.contains("rot") && t["rot"].size() >= 3) {
                    ent->transform.rotation = { t["rot"][0], t["rot"][1], t["rot"][2] };
                }
                if (t.contains("sca") && t["sca"].size() >= 3) {
                    ent->transform.scale = { t["sca"][0], t["sca"][1], t["sca"][2] };
                }
            }

            if (jEnt.contains("components") && jEnt["components"].is_array()) {
                for (const auto& jComp : jEnt["components"]) {
                    std::string type = jComp.value("type", "");
                    // Updated call site to pass jComp
                    Component* c = createComponentByType(ent, type, jComp);
                    if (c) {
                        c->deserialize(jComp);
                    }
                }
            }
        }
    }

    spdlog::info("Scene '{}' loaded from {}", m_activeScene->name, scenePath);
    return m_activeScene;
}