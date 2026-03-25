#pragma once

#include "engine/scene/scene.h"
#include <string>
#include <memory>

class SceneManager {
public:
    Scene* load(const std::string& scenePath);
    void unload();
    void save(const std::string& scenePath);

    void createScene(std::string name = "Scene") {
        unload();
        m_activeScene = std::make_unique<Scene>();
        m_activeScene->name = name;
    }

    Scene* getActiveScene() { return m_activeScene.get(); }
private:
    std::unique_ptr<Scene> m_activeScene;
};