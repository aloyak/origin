#pragma once

#include "engine/core/math.h"

#include <vector>

class btCollisionObject;
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

    void updateSingleAabb(btRigidBody* body);
    bool rayTest(const Vec3& origin,
                 const Vec3& direction,
                 float maxDistance,
                 btCollisionObject*& hitObject) const;

    static PhysicsWorld* getActive();
    // Only the engine's primary world should normally call this
    // Extra worlds must NOT call this, or they'll silently steal the "active" slot from the main world
    static void setActive(PhysicsWorld* world);

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

private:
    bool m_enabled = true;

    struct Data;
    Data* m_data = nullptr;
};