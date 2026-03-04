#pragma once

#include "engine/math.h"
#include <cmath>

class Transform {
public:
    Vec3 position = {0, 0, 0};
    Vec3 rotation = {0, 0, 0};
    Vec3 scale = {1, 1, 1};

    Vec3 forward() const {
        float yaw   = rotation.y * (3.14159265f / 180.0f);
        float pitch = rotation.x * (3.14159265f / 180.0f);
        return {
            cosf(yaw) * cosf(pitch),
            sinf(pitch),
            sinf(yaw) * cosf(pitch)
        };
    }

    Vec3 right() const {
        float yaw = (rotation.y - 90.0f) * (3.14159265f / 180.0f);
        return { cosf(yaw), 0, sinf(yaw) };
    }

    Vec3 up() const {
        Vec3 f = forward(), r = right();
        return {
            r.y*f.z - r.z*f.y,
            r.z*f.x - r.x*f.z,
            r.x*f.y - r.y*f.x
        };
    }
};