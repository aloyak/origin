#pragma once

#include "engine/core/math.h"

class PhysicsWorld {
public:
    PhysicsWorld();
    ~PhysicsWorld();

    void stepSimulation(float deltaTime);
    void setGravity(const Vec3& gravity);

private:
    struct Data;
    Data* m_data = nullptr;
};