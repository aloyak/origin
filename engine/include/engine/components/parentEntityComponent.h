#pragma once

#include "engine/components/entity.h"
#include "engine/components/component.h"
#include <memory>
#include <unordered_map>
#include <vector>

class SceneManager;

class ParentEntityComponent : public Component {
public:
    struct LocalTransform {
        Vec3 position = Vec3(0.0f, 0.0f, 0.0f);
        Quat rotation;
        Vec3 scale = Vec3(1.0f, 1.0f, 1.0f);
    };

    ParentEntityComponent(SceneManager& sceneManager) : m_sceneManager(sceneManager) {}

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<ParentEntityComponent>(*this);
    }

    void addChild(Entity* child, bool keepWorldTransform = true);
    void removeChild(Entity* child);
    void clearChildren();
    std::vector<Entity*> getChildren() const { return m_children; }

    void refreshLocalTransform(Entity* child);

    void update(float dt) override;

    SceneManager& getSceneManager() const { return m_sceneManager; }

    void serialize(nlohmann::json& j) const override;
    void deserialize(const nlohmann::json& j) override;

    bool applyPosition = true;
    bool applyRotation = true;
    bool applyScale = true;

private:
    LocalTransform computeLocalTransform(Entity* child) const;

    std::vector<Entity*> m_children;
    std::unordered_map<Entity*, LocalTransform> m_localTransforms;
    SceneManager& m_sceneManager;
};