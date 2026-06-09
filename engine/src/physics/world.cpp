#include "engine/physics/world.h"

#include "engine/components/entity.h"
#include "engine/components/rigidbodyComponent.h" 
#include "engine/render/camera.h"
#include "engine/scene/scene.h"

#include "engine/core/math.h"
#include "engine/core/transform.h"

#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>
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
} 

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
    if (!m_data || !m_enabled) return;
    m_data->world.stepSimulation(deltaTime, 10, 1.0f / 120.0f); // 120 FPS simulation
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

void PhysicsWorld::updateSingleAabb(btRigidBody* body) {
    if (m_data && body) {
        m_data->world.updateSingleAabb(body);
    }
}

bool PhysicsWorld::rayTest(const Vec3& origin,
                           const Vec3& direction,
                           float maxDistance,
                           btCollisionObject*& hitObject) const {
    hitObject = nullptr;

    if (!m_data || maxDistance <= 0.0f) {
        return false;
    }

    const Vec3 dir = direction.normalize();
    const btVector3 from(origin.x, origin.y, origin.z);
    const btVector3 to(
        origin.x + dir.x * maxDistance,
        origin.y + dir.y * maxDistance,
        origin.z + dir.z * maxDistance);

    btCollisionWorld::ClosestRayResultCallback callback(from, to);
    m_data->world.rayTest(from, to, callback);

    if (!callback.hasHit()) {
        return false;
    }

    hitObject = const_cast<btCollisionObject*>(callback.m_collisionObject);
    return hitObject != nullptr;
}

PhysicsWorld* PhysicsWorld::getActive() {
    return g_activeWorld;
}
