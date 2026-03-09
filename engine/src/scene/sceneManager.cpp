#include "engine/scene/sceneManager.h"
#include "spdlog/spdlog.h"
#include <nlohmann/json.hpp>

Scene* SceneManager::loadScene(const std::string& scenePath) { // TODO: implement loading from file
    m_activeScene = new Scene();
    return m_activeScene;
}

void SceneManager::unloadScene() {
    delete m_activeScene;
    m_activeScene = nullptr;
}

void SceneManager::saveScene(const std::string& scenePath) { // TODO: implement saving to file
}

Entity* SceneManager::createEntity(std::string name) {
    if (!m_activeScene) {
        spdlog::error("No active scene to create entity in!");
        return nullptr;
    }

    return m_activeScene->createEntity(name);
}