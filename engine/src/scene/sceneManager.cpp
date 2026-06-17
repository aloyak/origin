#include "engine/scene/sceneManager.h"

#include "engine/utils/logger.h"

#include "engine/components/cameraComponent.h"
#include "engine/components/rendererComponent.h"
#include "engine/components/skyboxComponent.h"
#include "engine/components/directionalLightComponent.h"
#include "engine/components/pointLightComponent.h"
#include "engine/components/rigidbodyComponent.h"
#include "engine/components/audioSourceComponent.h"
#include "engine/components/listenerComponent.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <memory>

#include "engine/utils/path.h"

using json = nlohmann::json;

namespace {
    bool tryReadVec3(const json& source, const char* key, Vec3& out) {
        if (!source.contains(key) || !source[key].is_array() || source[key].size() < 3) {
            return false;
        }

        const json& arr = source[key];
        if (!arr[0].is_number() || !arr[1].is_number() || !arr[2].is_number()) {
            return false;
        }

        out = Vec3(arr[0].get<float>(), arr[1].get<float>(), arr[2].get<float>());
        return true;
    }

    std::vector<std::string> readStringArray(const json& source, const char* key) {
        std::vector<std::string> out;
        if (!source.contains(key) || !source[key].is_array()) {
            return out;
        }

        for (const auto& value : source[key]) {
            if (!value.is_string()) {
                return {};
            }
            out.push_back(value.get<std::string>());
        }

        return out;
    }

    Component* createComponentByType(Entity* entity, const std::string& type, const json& jComp) {
        if (type == "RenderComponent") {
            const std::string modelPath = jComp.value("model", "");
            const std::string vertPath = jComp.value("vert", "assets/shaders/builtin/vert.glsl");
            const std::string fragPath = jComp.value("frag", "assets/shaders/builtin/frag.glsl");
            return entity->addComponent<RenderComponent>(modelPath, vertPath, fragPath);
        }
        if (type == "SkyboxComponent") {
            const std::vector<std::string> faces = readStringArray(jComp, "faces");
            if (faces.size() != 6) {
                Logger::warn("Skipping SkyboxComponent on entity '{}' due to invalid faces array.");
                return nullptr;
            }
            return entity->addComponent<SkyboxComponent>(faces);
        }
        if (type == "CameraComponent") return entity->addComponent<CameraComponent>(60.0f, 1.0f, 0.1f, 100.0f);
        if (type == "DirectionalLightComponent") return entity->addComponent<DirectionalLightComponent>();
        if (type == "PointLightComponent") return entity->addComponent<PointLightComponent>();
        if (type == "RigidbodyComponent") return entity->addComponent<RigidbodyComponent>();
        if (type == "AudioSourceComponent") return entity->addComponent<AudioSourceComponent>();
        if (type == "ListenerComponent") return entity->addComponent<ListenerComponent>();
        
        Logger::warn("Unknown component type on entity '" + entity->name + "'.");
        return nullptr;
    }
}

void SceneManager::unload() {
    if (m_activeScene) {
        m_activeScene.reset();
        Logger::info("Scene unloaded.");
    }
}

void SceneManager::save(const std::string& scenePath) {
    std::string resolvedScenePath = Path::resolve(scenePath).string();
    if (!m_activeScene) {
        Logger::warn("No active scene to save.");
        return;
    }

    json j;
    j["name"] = m_activeScene->name;
    j["ambientStrength"] = m_activeScene->getAmbientStrength();
    j["entities"] = json::array();

    for (const auto& entity : m_activeScene->getEntities()) {
        json jEnt;
        jEnt["name"] = entity->name;
        jEnt["tag"] = entity->getTag();
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

    std::ofstream file(resolvedScenePath);
    if (file.is_open()) {
        file << j.dump(4);
        Logger::info("Scene saved to: " + resolvedScenePath);
    } else {
        Logger::error("Failed to open file for saving: " + resolvedScenePath);
    }
}

Scene* SceneManager::load(const std::string& scenePath) {
    std::string resolvedScenePath = Path::resolve(scenePath).string();

    std::ifstream file(resolvedScenePath);
    if (!file.is_open()) {
        Logger::error("Could not open scene file: " + resolvedScenePath);
        return nullptr;
    }

    json j;
    try {
        file >> j;
    } catch (json::parse_error& e) {
        Logger::error("JSON parse error in " + resolvedScenePath + ": " + e.what());
        return nullptr;
    }

    if (!j.is_object()) {
        Logger::error("Scene root in " + resolvedScenePath + " must be a JSON object.");
        return nullptr;
    }

    auto loadedScene = std::make_unique<Scene>();
    loadedScene->name = j.value("name", "New Scene");

    if (j.contains("ambientStrength")) {
        loadedScene->setAmbientStrength(j["ambientStrength"].get<float>());
    }

    if (j.contains("entities") && j["entities"].is_array()) {
        for (const auto& jEnt : j["entities"]) {
            if (!jEnt.is_object()) {
                Logger::warn("Skipping malformed entity entry in scene '" + loadedScene->name + "'.");
                continue;
            }

            Entity* ent = loadedScene->createEntity(jEnt.value("name", "Entity"));
            ent->setTag(jEnt.value("tag", "Untagged"));

            if (jEnt.contains("transform") && jEnt["transform"].is_object()) {
                const auto& t = jEnt["transform"];
                Vec3 parsed;
                if (tryReadVec3(t, "pos", parsed)) ent->transform.position = parsed;
                if (tryReadVec3(t, "rot", parsed)) ent->transform.rotation = parsed;
                if (tryReadVec3(t, "sca", parsed)) ent->transform.scale = parsed;
            }

            if (jEnt.contains("components") && jEnt["components"].is_array()) {
                for (const auto& jComp : jEnt["components"]) {
                    if (!jComp.is_object()) {
                        Logger::warn("Skipping malformed component on entity '" + ent->name + "'.");
                        continue;
                    }

                    std::string type = jComp.value("type", "");
                    if (type.empty()) {
                        Logger::warn("Skipping component with missing type on entity '" + ent->name + "'.");
                        continue;
                    }

                    Component* c = createComponentByType(ent, type, jComp);
                    if (c) {
                        try {
                            c->deserialize(jComp);
                        } catch (const std::exception& e) {
                            Logger::error("Failed to deserialize component '" + type + "' on entity '" + ent->name + "': " + e.what());
                        }
                    }
                }
            }
        }
    }

    unload();
    m_activeScene = std::move(loadedScene);

    Logger::info("Scene '" + m_activeScene->name + "' loaded from " + resolvedScenePath);
    return m_activeScene.get();
}