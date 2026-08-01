#include "engine/components/rigidbodyComponent.h"
#include "engine/components/rendererComponent.h"
#include "engine/components/entity.h"

#include "engine/physics/world.h"
#include "engine/utils/logger.h"

#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>

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
    btQuaternion qX, qY, qZ;
    qX.setRotation(btVector3(1, 0, 0), toRadians(eulerDegrees.x));
    qY.setRotation(btVector3(0, 1, 0), toRadians(eulerDegrees.y));
    qZ.setRotation(btVector3(0, 0, 1), toRadians(eulerDegrees.z));
    return qZ * qY * qX;
}

Vec3 toEulerDegrees(const btQuaternion& q) {
    btScalar yaw, pitch, roll;
    btMatrix3x3(q).getEulerZYX(yaw, pitch, roll);
    return Vec3(toDegrees(roll), toDegrees(pitch), toDegrees(yaw));
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
    case RigidbodyComponent::ColliderType::Mesh:
        return "Mesh";
    case RigidbodyComponent::ColliderType::Box:
    default:
        return "Box";
    }
}

RigidbodyComponent::ColliderType colliderTypeFromString(const std::string& type) {
    if (type == "Sphere") return RigidbodyComponent::ColliderType::Sphere;
    if (type == "Capsule") return RigidbodyComponent::ColliderType::Capsule;
    if (type == "Mesh") return RigidbodyComponent::ColliderType::Mesh;
    
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
    j["freezeRotationX"] = m_freezeRotationX;
    j["freezeRotationY"] = m_freezeRotationY;
    j["freezeRotationZ"] = m_freezeRotationZ;
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
    setFreezeRotationX(j.value("freezeRotationX", false));
    setFreezeRotationY(j.value("freezeRotationY", false));
    setFreezeRotationZ(j.value("freezeRotationZ", false));

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
    markDirty();
}

void RigidbodyComponent::setBodyType(BodyType type) {
    if (m_bodyType == type) {
        return;
    }
    m_bodyType = type;

    m_dynamicMeshWarningLogged = false;
    m_missingMeshWarningLogged = false;

    markDirty();
}

void RigidbodyComponent::setColliderType(ColliderType type) {
    if (m_colliderType == type) {
        return;
    }
    m_colliderType = type;

    m_dynamicMeshWarningLogged = false;
    m_missingMeshWarningLogged = false;

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

void RigidbodyComponent::setFreezeRotationX(bool freeze) {
    m_freezeRotationX = freeze;
    if (m_body) {
        m_body->setAngularFactor(btVector3(
            m_freezeRotationX ? 0.0f : 1.0f,
            m_freezeRotationY ? 0.0f : 1.0f,
            m_freezeRotationZ ? 0.0f : 1.0f
        ));
    }
}

void RigidbodyComponent::setFreezeRotationY(bool freeze) {
    m_freezeRotationY = freeze;
    if (m_body) {
        m_body->setAngularFactor(btVector3(
            m_freezeRotationX ? 0.0f : 1.0f,
            m_freezeRotationY ? 0.0f : 1.0f,
            m_freezeRotationZ ? 0.0f : 1.0f
        ));
    }
}

void RigidbodyComponent::setFreezeRotationZ(bool freeze) {
    m_freezeRotationZ = freeze;
    if (m_body) {
        m_body->setAngularFactor(btVector3(
            m_freezeRotationX ? 0.0f : 1.0f,
            m_freezeRotationY ? 0.0f : 1.0f,
            m_freezeRotationZ ? 0.0f : 1.0f
        ));
    }
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

bool RigidbodyComponent::buildMeshColliderShape(const Vec3& safeScale) {
    if (!entity) return false;

    RenderComponent* renderComponent = entity->getComponent<RenderComponent>();
    if (!renderComponent) {
        if (!m_missingMeshWarningLogged) {
            Logger::warn("RigidbodyComponent: Mesh collider requested on entity '" + entity->name + "' but RenderComponent is missing.");
            m_missingMeshWarningLogged = true;
        }
        return false;
    }

    std::shared_ptr<Model> model = renderComponent->getModel();
    if (!model) {
        if (!m_missingMeshWarningLogged) {
            Logger::warn("RigidbodyComponent: Mesh collider requested on entity '" + entity->name + "' but model is not loaded.");
            m_missingMeshWarningLogged = true;
        }
        return false;
    }

    const auto& meshes = model->getMeshes();
    if (meshes.empty()) {
        if (!m_missingMeshWarningLogged) {
            Logger::warn("RigidbodyComponent: Mesh collider requested on entity '" + entity->name + "' but model has no meshes.");
            m_missingMeshWarningLogged = true;
        }
        return false;
    }

    delete m_triangleMesh;
    m_triangleMesh = new btTriangleMesh(true, true);

    std::size_t triangleCount = 0;
    for (const Mesh& mesh : meshes) {
        const auto& vertices = mesh.vertices;
        const auto& indices = mesh.indices;

        if (vertices.empty() || indices.size() < 3) continue;

        for (std::size_t i = 0; i + 2 < indices.size(); i += 3) {
            const unsigned int i0 = indices[i];
            const unsigned int i1 = indices[i + 1];
            const unsigned int i2 = indices[i + 2];

            if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size()) {
                continue;
            }

            const Vec3& p0 = vertices[i0].Position;
            const Vec3& p1 = vertices[i1].Position;
            const Vec3& p2 = vertices[i2].Position;

            const btVector3 v0(p0.x * safeScale.x, p0.y * safeScale.y, p0.z * safeScale.z);
            const btVector3 v1(p1.x * safeScale.x, p1.y * safeScale.y, p1.z * safeScale.z);
            const btVector3 v2(p2.x * safeScale.x, p2.y * safeScale.y, p2.z * safeScale.z);

            m_triangleMesh->addTriangle(v0, v1, v2, true);
            triangleCount++;
        }
    }

    if (triangleCount == 0) {
        delete m_triangleMesh;
        m_triangleMesh = nullptr;

        if (!m_missingMeshWarningLogged) {
            Logger::warn("RigidbodyComponent: Mesh collider requested on entity '" + entity->name + "' but no valid triangles were found.");
            m_missingMeshWarningLogged = true;
        }
        return false;
    }

    m_shape = new btBvhTriangleMeshShape(m_triangleMesh, true, true);

    m_missingMeshWarningLogged = false;
    return true;
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
    case ColliderType::Mesh: {
        if (m_bodyType == BodyType::Dynamic) {
            if (!m_dynamicMeshWarningLogged) {
                Logger::warn("RigidbodyComponent: Dynamic Mesh collider is not supported on entity '" + entity->name + "'. Use Static or Kinematic.");
                m_dynamicMeshWarningLogged = true;
            }
            break;
        }

        if (!buildMeshColliderShape(safeScale)) break;
        
        m_dynamicMeshWarningLogged = false;
        break;
    }
    case ColliderType::Box:
    default: {
        const btVector3 halfExtents(0.5f * scaledSize.x, 0.5f * scaledSize.y, 0.5f * scaledSize.z);
        m_shape = new btBoxShape(halfExtents);
        break;
    }
    }

    if (!m_shape) {
        m_dirty = false;
        return;
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
    m_body->setAngularFactor(btVector3(
        m_freezeRotationX ? 0.0f : 1.0f,
        m_freezeRotationY ? 0.0f : 1.0f,
        m_freezeRotationZ ? 0.0f : 1.0f
    ));

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

    if (!isStatic && !isKinematic) {
        const float smallestExtent = std::min(scaledSize.x, std::min(scaledSize.y, scaledSize.z));
        m_body->setCcdMotionThreshold(smallestExtent * 0.5f);
        m_body->setCcdSweptSphereRadius(smallestExtent * 0.4f);
    }

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

    delete m_triangleMesh;
    m_triangleMesh = nullptr;
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

void RigidbodyComponent::applyForce(const Vec3& force) {
    if (!m_body || m_bodyType != BodyType::Dynamic) {
        return;
    }

    m_body->activate(true);
    m_body->applyCentralForce(btVector3(force.x, force.y, force.z));
}

void RigidbodyComponent::applyImpulse(const Vec3& impulse) {
    if (!m_body || m_bodyType != BodyType::Dynamic) {
        return;
    }

    m_body->activate(true);
    m_body->applyCentralImpulse(btVector3(impulse.x, impulse.y, impulse.z));
}

void RigidbodyComponent::applyTorque(const Vec3& torque) {
    if (!m_body || m_bodyType != BodyType::Dynamic) {
        return;
    }

    m_body->activate(true);
    m_body->applyTorque(btVector3(torque.x, torque.y, torque.z));
}


void RigidbodyComponent::teleport(const Vec3& position, const Vec3& eulerDegrees) {
    if (!entity) return;

    entity->transform.position = position;
    entity->transform.rotation = eulerDegrees;

    if (m_body) {
        syncBodyFromTransform();

        m_body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
        m_body->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
        m_body->activate(true);
    }
}

void RigidbodyComponent::shiftOrigin(const Vec3& delta) {
    if (!entity) return;
    entity->transform.position = entity->transform.position + delta;

    if (m_body) {
        btTransform t = m_body->getWorldTransform();
        t.setOrigin(t.getOrigin() + btVector3(delta.x, delta.y, delta.z));
        m_body->setWorldTransform(t);
        if (m_body->getMotionState())
            m_body->getMotionState()->setWorldTransform(t);
        if (m_world) m_world->updateSingleAabb(m_body);
    }
}

Vec3 RigidbodyComponent::getLinearVelocity() const {
    if (!m_body) return Vec3(0.0f, 0.0f, 0.0f);
    const btVector3 vel = m_body->getLinearVelocity();
    return Vec3(vel.x(), vel.y(), vel.z());
}

void RigidbodyComponent::setLinearVelocity(const Vec3& velocity) {
    if (!m_body) return;
    m_body->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
}

Vec3 RigidbodyComponent::getAngularVelocity() const {
    if (!m_body) return Vec3(0.0f, 0.0f, 0.0f);
    const btVector3 vel = m_body->getAngularVelocity();
    return Vec3(vel.x(), vel.y(), vel.z());
}

void RigidbodyComponent::setAngularVelocity(const Vec3& velocity) {
    if (!m_body) return;
    m_body->setAngularVelocity(btVector3(velocity.x, velocity.y, velocity.z));
}