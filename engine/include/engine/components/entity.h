#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <string>

#include "engine/components/component.h"
#include "engine/transform.h"

class Engine;
class Camera;

class Entity {
public:
    std::string name = "Entity";
    Transform transform;

    template<typename T, typename... Args>
    T* addComponent(Args&&... args) {
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        comp->entity = this;
        T* ptr = comp.get();
        m_components[std::type_index(typeid(T))] = std::move(comp);
        return ptr;
    }

    template<typename T>
    T* getComponent() {
        auto it = m_components.find(std::type_index(typeid(T)));
        if (it != m_components.end())
            return static_cast<T*>(it->second.get());
        return nullptr;
    }

    template<typename T>
    bool hasComponent() const {
        return m_components.count(std::type_index(typeid(T))) > 0;
    }

    template<typename T>
    void removeComponent() {
        m_components.erase(std::type_index(typeid(T)));
    }

    void update(float dt) {
        for (auto& [type, comp] : m_components)
            comp->update(dt);
    }

    void render(Engine& engine, const Camera& camera, const Transform& cameraTransform) {
        for (auto& [type, comp] : m_components)
            comp->render(engine, camera, cameraTransform);
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;
};