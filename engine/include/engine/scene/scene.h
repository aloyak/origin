#pragma once

#include "engine/components/entity.h"

#include <vector>
#include <string>

class Scene {
public:
    std::string name;

    void update();
private:
    std::vector<std::unique_ptr<Entity>> m_entities;
};