#pragma once

#include "engine/components/entity.h"

#include <vector>
#include <string>
#include <memory>

class Scene {
public:
    std::string name = "Scene";

    Entity* createEntity(std::string name = "Entity");
    void destroyEntity(Entity* entity);
    void addEntity(std::unique_ptr<Entity> entity);

    void update(float deltaTime);
    void render(Engine& engine, const Camera& camera, const Transform& cameraTransform);

    const std::vector<std::unique_ptr<Entity>>& getEntities() const { return m_entities; }

private:
    std::vector<std::unique_ptr<Entity>> m_entities;
};