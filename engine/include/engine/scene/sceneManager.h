#pragma once

#include "engine/scene/scene.h"

#include <string>

class SceneManager {
public:
    static void loadScene(const std::string& scenePath);
    static void saveScene(const std::string& scenePath);

private:
    static Scene* s_activeScene;
};