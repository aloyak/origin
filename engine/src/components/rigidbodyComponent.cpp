#include "engine/components/rigidbodyComponent.h"

#include "engine/components/entity.h"
#include "engine/physics/world.h"
#include "engine/utils/logger.h"

#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>

#include <BulletDynamics/Dynamics/btRigidBody.h>

#include <LinearMath/btDefaultMotionState.h>
#include <LinearMath/btMatrix3x3.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btTransform.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <string>

namespace {
float toRadians(float degrees) {
    return degrees * 0.01745329251994329577f;
}

float toDegrees(float radians) {
    return radians * 57.295779513082320876f;
}

btQuaternion toBulletQuaternion(const Vec3& eulerDegrees) {
    btQuaternion qY, qX, qZ;
    qY.setRotation(btVector3(0, 1, 0), toRadians(eulerDegrees.y));
    qX.setRotation(btVector3(1, 0, 0), toRadians(eulerDegrees.x));
    qZ.setRotation(btVector3(0, 0, 1), toRadians(eulerDegrees.z));
    return qY * qX * qZ;
}

Vec3 toEulerDegrees(const btQuaternion& q) {
    btScalar yaw, pitch, roll;
    btMatrix3x3(q).getEulerZYX(roll, pitch, yaw);
    return Vec3(toDegrees(pitch), toDegrees(yaw), toDegrees(roll));
}

std::string bodyTypeToString(RigidbodyComponent::BodyType type) {
    switch (type) {
    case RigidbodyComponent::BodyType::Static:
        return "Static";
    case RigidbodyComponent::BodyType::Kinematic:
        return "Kinematic";
    case RigidbodyComponent::BodyType::Dynamic:
    default:
        return "Dynamic";
    }
}

RigidbodyComponent::BodyType bodyTypeFromString(const std::string& type) {
    if (type == "Static") return RigidbodyComponent::BodyType::Static;
    if (type == "Kinematic") return RigidbodyComponent::BodyType::Kinematic;
    
    return RigidbodyComponent::BodyType::Dynamic;
}

std::string colliderTypeToString(RigidbodyComponent::ColliderType type) {
    switch (type) {
    case RigidbodyComponent::ColliderType::Sphere:
        return "Sphere";
    case RigidbodyComponent::ColliderType::Capsule:
        return "Capsule";
    case RigidbodyComponent::ColliderType::Box:
    default:
        return "Box";
    }
}

RigidbodyComponent::ColliderType colliderTypeFromString(const std::string& type) {
    if (type == "Sphere") {
        return RigidbodyComponent::ColliderType::Sphere;
    }
    if (type == "Capsule") {
        return RigidbodyComponent::ColliderType::Capsule;
    }
    return RigidbodyComponent::ColliderType::Box;
}

Vec3 scaledAbsSize(const Vec3& base, const Vec3& scale) {
    return Vec3(
        std::max(0.01f, std::fabs(base.x * scale.x)),
        std::max(0.01f, std::fabs(base.y * scale.y)),
        std::max(0.01f, std::fabs(base.z * scale.z)));
}
} // namespace

RigidbodyComponent::RigidbodyComponent(PhysicsWorld* world)
    : m_world(world) {}

RigidbodyComponent::~RigidbodyComponent() {
    destroyBody();
}

std::unique_ptr<Component> RigidbodyComponent::clone() const {
    auto copy = std::make_unique<RigidbodyComponent>(m_world);
    nlohmann::json j;
    serialize(j);
    copy->deserialize(j);
    copy->isEnabled = isEnabled;
    return copy;
}

void RigidbodyComponent::update(float /*dt*/) {
    if (!entity) return;
    if (!m_world) m_world = PhysicsWorld::getActive();
    if (!m_world) return;

    const Vec3 currentScale = entity->transform.scale;
    const float scaleEpsilon = 0.0001f;
    if (std::fabs(currentScale.x - m_lastTrackedScale.x) > scaleEpsilon ||
        std::fabs(currentScale.y - m_lastTrackedScale.y) > scaleEpsilon ||
        std::fabs(currentScale.z - m_lastTrackedScale.z) > scaleEpsilon) {
        m_lastTrackedScale = currentScale;
        markDirty();
    }

    if (m_dirty || !m_body) rebuildBody();

    if (!m_world->isEnabled()) {
        syncBodyFromTransform();
        return;
    }

    if (m_bodyType == BodyType::Kinematic) {
        syncBodyFromTransform();
    } else if (m_bodyType == BodyType::Dynamic) {
        syncTransformFromBody();
    }
}

void RigidbodyComponent::serialize(nlohmann::json& j) const {
    j["type"] = "RigidbodyComponent";
    j["bodyType"] = bodyTypeToString(m_bodyType);
    j["colliderType"] = colliderTypeToString(m_colliderType);
    j["mass"] = m_mass;
    j["friction"] = m_friction;
    j["restitution"] = m_restitution;
    j["linearDamping"] = m_linearDamping;
    j["angularDamping"] = m_angularDamping;
    j["useGravity"] = m_useGravity;
    j["colliderSize"] = {m_colliderSize.x, m_colliderSize.y, m_colliderSize.z};
}

void RigidbodyComponent::deserialize(const nlohmann::json& j) {
    setBodyType(bodyTypeFromString(j.value("bodyType", std::string("Dynamic"))));
    setColliderType(colliderTypeFromString(j.value("colliderType", std::string("Box"))));
    setMass(j.value("mass", 1.0f));
    setFriction(j.value("friction", 0.5f));
    setRestitution(j.value("restitution", 0.0f));
    setLinearDamping(j.value("linearDamping", 0.02f));
    setAngularDamping(j.value("angularDamping", 0.05f));
    setUseGravity(j.value("useGravity", true));

    if (j.contains("colliderSize") && j["colliderSize"].is_array() && j["colliderSize"].size() >= 3) {
        const auto& size = j["colliderSize"];
        if (size[0].is_number() && size[1].is_number() && size[2].is_number()) {
            setColliderSize(Vec3(size[0].get<float>(), size[1].get<float>(), size[2].get<float>()));
        }
    }
}

void RigidbodyComponent::setPhysicsWorld(PhysicsWorld* world) {
    if (m_world == world) {
        return;
    }

    destroyBody();
    m_world = world;
    m_dirty = true;
}

void RigidbodyComponent::setBodyType(BodyType type) {
    if (m_bodyType == type) {
        return;
    }
    m_bodyType = type;
    markDirty();
}

void RigidbodyComponent::setColliderType(ColliderType type) {
    if (m_colliderType == type) {
        return;
    }
    m_colliderType = type;
    markDirty();
}

void RigidbodyComponent::setMass(float mass) {
    m_mass = std::max(0.0f, mass);
    markDirty();
}

void RigidbodyComponent::setFriction(float friction) {
    m_friction = std::max(0.0f, friction);
    if (m_body) {
        m_body->setFriction(m_friction);
    }
}

void RigidbodyComponent::setRestitution(float restitution) {
    m_restitution = std::max(0.0f, restitution);
    if (m_body) {
        m_body->setRestitution(m_restitution);
    }
}

void RigidbodyComponent::setLinearDamping(float damping) {
    m_linearDamping = std::max(0.0f, damping);
    if (m_body) {
        m_body->setDamping(m_linearDamping, m_angularDamping);
    }
}

void RigidbodyComponent::setAngularDamping(float damping) {
    m_angularDamping = std::max(0.0f, damping);
    if (m_body) {
        m_body->setDamping(m_linearDamping, m_angularDamping);
    }
}

void RigidbodyComponent::setUseGravity(bool useGravity) {
    m_useGravity = useGravity;
    if (m_body) {
        if (m_useGravity) {
            m_body->setFlags(m_body->getFlags() & ~BT_DISABLE_WORLD_GRAVITY);
            const Vec3 worldGravity = m_world ? m_world->getGravity() : Vec3(0.0f, -9.81f, 0.0f);
            m_body->setGravity(btVector3(worldGravity.x, worldGravity.y, worldGravity.z));
        } else {
            m_body->setFlags(m_body->getFlags() | BT_DISABLE_WORLD_GRAVITY);
            m_body->setGravity(btVector3(0.0f, 0.0f, 0.0f));
        }
    }
}

void RigidbodyComponent::setColliderSize(const Vec3& size) {
    m_colliderSize.x = std::max(0.01f, std::fabs(size.x));
    m_colliderSize.y = std::max(0.01f, std::fabs(size.y));
    m_colliderSize.z = std::max(0.01f, std::fabs(size.z));
    markDirty();
}

void RigidbodyComponent::resetMotion() {
    if (!m_body) return;

    m_body->clearForces();
    m_body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
    m_body->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
    m_body->setInterpolationLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
    m_body->setInterpolationAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
    m_body->activate(true);
}

void RigidbodyComponent::markDirty() {
    m_dirty = true;
}

void RigidbodyComponent::rebuildBody() {
    destroyBody();

    if (!m_world || !entity) {
        return;
    }

    Vec3 safeScale = entity->transform.scale;
    bool hasNegativeScale = false;
    if (safeScale.x < 0.0f || safeScale.y < 0.0f || safeScale.z < 0.0f) {
        hasNegativeScale = true;
        safeScale.x = std::fabs(safeScale.x);
        safeScale.y = std::fabs(safeScale.y);
        safeScale.z = std::fabs(safeScale.z);
    }
    if (safeScale.x < 0.01f) safeScale.x = 0.01f;
    if (safeScale.y < 0.01f) safeScale.y = 0.01f;
    if (safeScale.z < 0.01f) safeScale.z = 0.01f;
    if (hasNegativeScale) {
        Logger::warn("RigidbodyComponent: Entity '" + entity->name + "' has negative scale. Flipping to absolute values for physics. Visual model will not match physics bounds.");
    }

    const Vec3 scaledSize = scaledAbsSize(m_colliderSize, safeScale);

    switch (m_colliderType) {
    case ColliderType::Sphere: {
        const float radius = std::max(0.01f, 0.5f * std::max(scaledSize.x, std::max(scaledSize.y, scaledSize.z)));
        m_shape = new btSphereShape(radius);
        break;
    }
    case ColliderType::Capsule: {
        const float radius = std::max(0.01f, 0.5f * std::max(scaledSize.x, scaledSize.z));
        const float height = std::max(0.01f, scaledSize.y - 2.0f * radius);
        m_shape = new btCapsuleShape(radius, height);
        break;
    }
    case ColliderType::Box:
    default: {
        const btVector3 halfExtents(0.5f * scaledSize.x, 0.5f * scaledSize.y, 0.5f * scaledSize.z);
        m_shape = new btBoxShape(halfExtents);
        m_shape->setMargin(0.01f);
        break;
    }
    }

    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(entity->transform.position.x, entity->transform.position.y, entity->transform.position.z));
    startTransform.setRotation(toBulletQuaternion(entity->transform.rotation));

    m_motionState = new btDefaultMotionState(startTransform);

    const bool isStatic = m_bodyType == BodyType::Static;
    const bool isKinematic = m_bodyType == BodyType::Kinematic;

    const float usedMass = (isStatic || isKinematic) ? 0.0f : std::max(0.0f, m_mass);

    btVector3 localInertia(0.0f, 0.0f, 0.0f);
    if (usedMass > 0.0f) {
        m_shape->calculateLocalInertia(usedMass, localInertia);
    }

    btRigidBody::btRigidBodyConstructionInfo bodyInfo(usedMass, m_motionState, m_shape, localInertia);
    bodyInfo.m_friction = m_friction;
    bodyInfo.m_restitution = m_restitution;

    m_body = new btRigidBody(bodyInfo);
    m_body->setDamping(m_linearDamping, m_angularDamping);

    if (isStatic) {
        m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    }

    if (isKinematic) {
        m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        m_body->setActivationState(DISABLE_DEACTIVATION);
    }

    if (!m_useGravity) {
        m_body->setFlags(m_body->getFlags() | BT_DISABLE_WORLD_GRAVITY);
        m_body->setGravity(btVector3(0.0f, 0.0f, 0.0f));
    }

    if (!isStatic && !isKinematic) {
        m_body->setSleepingThresholds(0.8f, 1.0f);
    }

    m_world->addRigidBody(m_body);
    
    m_body->setCcdMotionThreshold(1e-7);
    m_body->setCcdSweptSphereRadius(scaledSize.x * 0.2f); // m_colliderSize before

    m_registered = true;
    m_dirty = false;
    

    m_lastTrackedScale = entity->transform.scale;
}

void RigidbodyComponent::destroyBody() {
    if (m_world && m_registered && m_body) {
        m_world->removeRigidBody(m_body);
    }

    m_registered = false;

    delete m_body;
    m_body = nullptr;

    delete m_motionState;
    m_motionState = nullptr;

    delete m_shape;
    m_shape = nullptr;
}

void RigidbodyComponent::syncTransformFromBody() const {
    if (!entity || !m_body) {
        return;
    }

    btTransform worldTransform;
    if (m_body->getMotionState()) {
        m_body->getMotionState()->getWorldTransform(worldTransform);
    } else {
        worldTransform = m_body->getWorldTransform();
    }

    const btVector3 origin = worldTransform.getOrigin();
    entity->transform.position = Vec3(origin.x(), origin.y(), origin.z());

    entity->transform.rotation = toEulerDegrees(worldTransform.getRotation());
}

void RigidbodyComponent::syncBodyFromTransform() const {
    if (!entity || !m_body) {
        return;
    }

    btTransform worldTransform;
    worldTransform.setIdentity();
    worldTransform.setOrigin(btVector3(entity->transform.position.x, entity->transform.position.y, entity->transform.position.z));
    worldTransform.setRotation(toBulletQuaternion(entity->transform.rotation));

    m_body->setWorldTransform(worldTransform);
    if (m_body->getMotionState()) {
        m_body->getMotionState()->setWorldTransform(worldTransform);
    }

    if (m_world) {
        m_world->updateSingleAabb(m_body);
    }
}
