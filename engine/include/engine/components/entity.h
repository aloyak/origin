#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

#include "engine/components/component.h"
#include "engine/core/transform.h"

class Engine;
class Camera;

class Entity {
public:
    std::string name = "Entity";
    Transform transform;

    template<typename T, typename... Args>
    T* addComponent(Args&&... args) {
        const std::type_index type = std::type_index(typeid(T));
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        comp->entity = this;
        T* ptr = comp.get();

        if (m_components.find(type) == m_components.end()) {
            m_componentOrder.push_back(type);
        }

        m_components[type] = std::move(comp);
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
        const std::type_index type = std::type_index(typeid(T));
        m_components.erase(type);
        m_componentOrder.erase(
            std::remove(m_componentOrder.begin(), m_componentOrder.end(), type),
            m_componentOrder.end()
        );
    }

    void update(float dt) {
        for (const auto& type : m_componentOrder) {
            auto it = m_components.find(type);
            if (it != m_components.end())
                it->second->update(dt);
        }
    }

    void render(Renderer& renderer, const Camera& camera, const Transform& cameraTransform) {
        for (const auto& type : m_componentOrder) {
            auto it = m_components.find(type);
            if (it != m_components.end())
                it->second->render(renderer, camera, cameraTransform);
        }
    }

    const std::unordered_map<std::type_index, std::unique_ptr<Component>>& getComponents() const { 
        return m_components; 
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;
    std::vector<std::type_index> m_componentOrder;
};