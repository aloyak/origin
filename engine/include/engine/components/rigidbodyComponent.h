#pragma once

#include "engine/components/component.h"

class PhysicsWorld;
class btCollisionShape;
class btDefaultMotionState;
class btRigidBody;
class btTriangleMesh;

class RigidbodyComponent : public Component {
public:
    enum class BodyType {
        Dynamic,
        Static,
        Kinematic,
    };

    enum class ColliderType {
        Box,
        Sphere,
        Capsule,
        Mesh
    };

    explicit RigidbodyComponent(PhysicsWorld* world = nullptr);
    ~RigidbodyComponent() override;

    void update(float dt) override;

    std::unique_ptr<Component> clone() const override;

    void serialize(nlohmann::json& j) const override;
    void deserialize(const nlohmann::json& j) override;

    void setPhysicsWorld(PhysicsWorld* world);
    PhysicsWorld* getPhysicsWorld() const { return m_world; }

    void setBodyType(BodyType type);
    BodyType getBodyType() const { return m_bodyType; }

    void setColliderType(ColliderType type);
    ColliderType getColliderType() const { return m_colliderType; }

    void setMass(float mass);
    float getMass() const { return m_mass; }

    void setFriction(float friction);
    float getFriction() const { return m_friction; }

    void setRestitution(float restitution);
    float getRestitution() const { return m_restitution; }

    void setLinearDamping(float damping);
    float getLinearDamping() const { return m_linearDamping; }

    void setAngularDamping(float damping);
    float getAngularDamping() const { return m_angularDamping; }

    void setUseGravity(bool useGravity);
    bool getUseGravity() const { return m_useGravity; }

    void setColliderSize(const Vec3& size);
    Vec3 getColliderSize() const { return m_colliderSize; }

    
    btRigidBody* getRigidBody() const { return m_body; }

    void resetMotion();

private:
    void markDirty();
    void rebuildBody();
    void destroyBody();
    void syncBodyFromTransform() const;
    void syncTransformFromBody() const;

    PhysicsWorld* m_world = nullptr;

    btCollisionShape* m_shape = nullptr;
    btDefaultMotionState* m_motionState = nullptr;
    btRigidBody* m_body = nullptr;
    btTriangleMesh* m_triangleMesh = nullptr;
    bool m_registered = false;
    bool m_dirty = true;

    bool buildMeshColliderShape(const Vec3& safeScale);
    bool m_missingMeshWarningLogged = false;
    bool m_dynamicMeshWarningLogged = false;

    BodyType m_bodyType = BodyType::Dynamic;
    ColliderType m_colliderType = ColliderType::Box;
    float m_mass = 1.0f;
    float m_friction = 0.5f;
    float m_restitution = 0.0f;
    float m_linearDamping = 0.02f;
    float m_angularDamping = 0.05f;
    bool m_useGravity = true;
    Vec3 m_colliderSize = Vec3(1.0f, 1.0f, 1.0f);
    Vec3 m_lastTrackedScale = Vec3(1.0f, 1.0f, 1.0f);
};
