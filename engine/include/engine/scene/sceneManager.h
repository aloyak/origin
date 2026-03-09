#pragma once

#include "engine/scene/scene.h"
#include <string>

class SceneManager {
public:
    Scene* loadScene(const std::string& scenePath);
    void unloadScene();
    void saveScene(const std::string& scenePath);

    Scene* getActiveScene() { return m_activeScene; }

    Entity* createEntity(std::string name = "Entity");

private:
    Scene* m_activeScene = nullptr;
};