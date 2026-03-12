#include "engine/scene/scene.h"
#include <algorithm>

Entity* Scene::createEntity(std::string name) {
    m_entities.push_back(std::make_unique<Entity>());
    m_entities.back()->name = name;
    return m_entities.back().get();
}

void Scene::addEntity(std::unique_ptr<Entity> entity) {
    m_entities.push_back(std::move(entity));
}

void Scene::destroyEntity(Entity* entity) {
    m_entities.erase(
        std::remove_if(m_entities.begin(), m_entities.end(),
            [entity](const std::unique_ptr<Entity>& e) { return e.get() == entity; }),
        m_entities.end()
    );
}

void Scene::update(float deltaTime) {
    for (auto& entity : m_entities) {
        entity->update(deltaTime);
    }
}

void Scene::render(Engine& engine, const Camera& camera, const Transform& cameraTransform) {
    for (auto& entity : m_entities) {
        entity->render(engine, camera, cameraTransform);
    }
}