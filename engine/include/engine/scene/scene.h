#pragma once

#include "engine/components/entity.h"
#include "engine/render/render.h"

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
    void render(Renderer& renderer, const Camera& camera, const Transform& cameraTransform);

    const std::vector<std::unique_ptr<Entity>>& getEntities() const { return m_entities; }
    std::unique_ptr<Entity>& getEntity(size_t index) { return m_entities[index]; }
    std::unique_ptr<Entity>& getEntityByName(const std::string name);

    void setAmbientStrength(float strength) { m_ambientStrength = strength; }
    float getAmbientStrength() const { return m_ambientStrength; }
private:
    float m_ambientStrength = 0.25f;
    std::vector<std::unique_ptr<Entity>> m_entities;
};