#pragma once

#include "engine/core/math.h"

#include <vector>

class btRigidBody;
class Camera;
class Entity;
class Scene;
class Transform;

class PhysicsWorld {
public:
    PhysicsWorld();
    ~PhysicsWorld();

    void stepSimulation(float deltaTime);
    void setGravity(const Vec3& gravity);
    Vec3 getGravity() const;
    void addRigidBody(btRigidBody* body);
    void removeRigidBody(btRigidBody* body);

    static PhysicsWorld* getActive();

    Entity* raycastScene(const Vec3& origin,
                         const Vec3& direction,
                         Scene& scene,
                         float maxDistance = 10000.0f) const;

    Entity* raycastEntities(const Vec3& origin,
                            const Vec3& direction,
                            const std::vector<Entity*>& entities,
                            float maxDistance = 10000.0f) const;

    Entity* raycastScreenPoint(const Vec2& screenPoint,
                               const Vec2& viewportSize,
                               const Camera& camera,
                               const Transform& cameraTransform,
                               Scene& scene,
                               float maxDistance = 10000.0f) const;

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

private:
    bool m_enabled = true;

    struct Data;
    Data* m_data = nullptr;
};