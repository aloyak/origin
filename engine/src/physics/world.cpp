#include "engine/physics/world.h"

#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <LinearMath/btVector3.h>

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
}

PhysicsWorld::~PhysicsWorld() {
    delete m_data;
}

void PhysicsWorld::stepSimulation(float deltaTime) {
    if (!m_data) {
        return;
    }
    m_data->world.stepSimulation(deltaTime, 4, 1.0f / 60.0f);
}

void PhysicsWorld::setGravity(const Vec3& gravity) {
    if (!m_data) {
        return;
    }
    m_data->world.setGravity(btVector3(gravity.x, gravity.y, gravity.z));
}