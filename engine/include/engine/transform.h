#pragma once

#include "engine/math.h"

class Transform {
public:
    Vec3 position;
    Vec3 rotation; // Euler angles in degrees
    Vec3 scale;

    Transform() : position(0,0,0), rotation(0,0,0), scale(1,1,1) {}
};