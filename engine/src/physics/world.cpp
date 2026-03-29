#include "engine/physics/world.h"

#include "engine/components/entity.h"
#include "engine/render/camera.h"
#include "engine/scene/scene.h"
#include "engine/core/transform.h"

#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>

#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <LinearMath/btVector3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace {
PhysicsWorld* g_activeWorld = nullptr;

float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float length(const Vec3& v) {
    return std::sqrt(dot(v, v));
}

Vec3 normalize(const Vec3& v) {
    const float len = length(v);
    if (len <= 0.000001f) {
        return Vec3(0.0f, 0.0f, -1.0f);
    }
    return Vec3(v.x / len, v.y / len, v.z / len);
}

bool intersectSphere(const Vec3& origin,
                     const Vec3& direction,
                     const Vec3& center,
                     float radius,
                     float maxDistance,
                     float& outDistance) {
    const Vec3 oc = origin - center;
    const float b = dot(oc, direction);
    const float c = dot(oc, oc) - radius * radius;
    const float discriminant = b * b - c;
    if (discriminant < 0.0f) {
        return false;
    }

    const float sqrtDisc = std::sqrt(discriminant);
    float t = -b - sqrtDisc;
    if (t < 0.0f) {
        t = -b + sqrtDisc;
    }

    if (t < 0.0f || t > maxDistance) {
        return false;
    }

    outDistance = t;
    return true;
}
} // namespace

struct PhysicsWorld::Data {
    btDefaultCollisionConfiguration collisionConfig;
    btCollisionDispatcher dispatcher;
    btDbvtBroadphase broadphase;
    btSequentialImpulseConstraintSolver solver;
    btDiscreteDynamicsWorld world;

    Data()
        : dispatcher(&collisionConfig),
          world(&dispatcher, &broadphase, &solver, &collisionConfig) {
        world.setGravity(btVector3(0.0f, -9.81f, 0.0f));
    }
};

PhysicsWorld::PhysicsWorld() {
    m_data = new Data();
    g_activeWorld = this;
}

PhysicsWorld::~PhysicsWorld() {
    if (g_activeWorld == this) g_activeWorld = nullptr;
    delete m_data;
}

void PhysicsWorld::stepSimulation(float deltaTime) {
    if (!m_data || !m_enabled) {
        return;
    }
    m_data->world.stepSimulation(deltaTime, 4, 1.0f / 60.0f); // 60 FPS, max 4 substeps for better accuracy
}

void PhysicsWorld::setGravity(const Vec3& gravity) {
    if (!m_data) {
        return;
    }
    m_data->world.setGravity(btVector3(gravity.x, gravity.y, gravity.z));
}

Vec3 PhysicsWorld::getGravity() const {
    if (!m_data) {
        return Vec3(0.0f, -9.81f, 0.0f);
    }

    const btVector3 gravity = m_data->world.getGravity();
    return Vec3(gravity.x(), gravity.y(), gravity.z());
}

void PhysicsWorld::addRigidBody(btRigidBody* body) {
    if (!m_data || !body) {
        return;
    }
    m_data->world.addRigidBody(body);
}

void PhysicsWorld::removeRigidBody(btRigidBody* body) {
    if (!m_data || !body) {
        return;
    }
    m_data->world.removeRigidBody(body);
}

PhysicsWorld* PhysicsWorld::getActive() {
    return g_activeWorld;
}

Entity* PhysicsWorld::raycastEntities(const Vec3& origin,
                                      const Vec3& direction,
                                      const std::vector<Entity*>& entities,
                                      float maxDistance) const {
    const Vec3 rayDir = normalize(direction);

    Entity* nearestEntity = nullptr;
    float nearestDistance = maxDistance;

    for (Entity* entity : entities) {
        if (!entity) {
            continue;
        }

        const Vec3 scale = entity->transform.scale;
        const float maxScaleAxis = std::max(std::fabs(scale.x), std::max(std::fabs(scale.y), std::fabs(scale.z)));
        // Use a more generous radius for better picking: min 1.0 unit
        const float radius = std::max(1.0f, 0.6f * maxScaleAxis);

        float hitDistance = 0.0f;
        if (intersectSphere(origin, rayDir, entity->transform.position, radius, maxDistance, hitDistance)) {
            if (hitDistance < nearestDistance) {
                nearestDistance = hitDistance;
                nearestEntity = entity;
            }
        }
    }

    return nearestEntity;
}

Entity* PhysicsWorld::raycastScene(const Vec3& origin,
                                   const Vec3& direction,
                                   Scene& scene,
                                   float maxDistance) const {
    std::vector<Entity*> entities;
    entities.reserve(scene.getEntities().size());

    for (const auto& entity : scene.getEntities()) {
        entities.push_back(entity.get());
    }

    return raycastEntities(origin, direction, entities, maxDistance);
}

Entity* PhysicsWorld::raycastScreenPoint(const Vec2& screenPoint,
                                         const Vec2& viewportSize,
                                         const Camera& camera,
                                         const Transform& cameraTransform,
                                         Scene& scene,
                                         float maxDistance) const {
    if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f) {
        return nullptr;
    }

    const glm::mat4 view = *static_cast<const glm::mat4*>(camera.getViewMatrix(cameraTransform));
    const glm::mat4 projection = *static_cast<const glm::mat4*>(camera.getProjectionMatrix());
    
    // Construct ray manually using inverse matrices for better clarity
    const glm::mat4 viewInv = glm::inverse(view);
    const glm::mat4 projInv = glm::inverse(projection);

    // Convert screen coordinates to normalized device coordinates [-1, 1]
    const float normalizedX = (2.0f * screenPoint.x) / viewportSize.x - 1.0f;
    const float normalizedY = 1.0f - (2.0f * screenPoint.y) / viewportSize.y;  // Flip Y for OpenGL

    // Construct ray in view space
    glm::vec4 rayClip(normalizedX, normalizedY, -1.0f, 1.0f);
    glm::vec4 rayView = projInv * rayClip;
    rayView = glm::vec4(rayView.x, rayView.y, -1.0f, 0.0f);  // Direction, not position

    // Transform to world space
    glm::vec4 rayWorld = viewInv * rayView;
    glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld.x, rayWorld.y, rayWorld.z));
    
    // Ray origin is camera position
    Vec3 rayOrigin(cameraTransform.position.x, cameraTransform.position.y, cameraTransform.position.z);
    Vec3 rayDir(rayDirection.x, rayDirection.y, rayDirection.z);

    return raycastScene(rayOrigin, rayDir, scene, maxDistance);
}