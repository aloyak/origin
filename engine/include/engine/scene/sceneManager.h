#pragma once

#include "engine/scene/scene.h"
#include <string>

class SceneManager {
public:
    Scene* load(const std::string& scenePath);
    void unload();
    void save(const std::string& scenePath);

    Scene* getActiveScene() { return m_activeScene; }

    Entity* createEntity(std::string name = "Entity");

private:
    Scene* m_activeScene = nullptr;
};