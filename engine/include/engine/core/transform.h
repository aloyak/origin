#pragma once

#include "engine/core/math.h"
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
        return cross(up(), forward()).normalize();
    }

    Vec3 up() const {
        Vec3 f = forward();
        Vec3 worldUp(0.0f, 1.0f, 0.0f);
        float roll = rotation.z * (3.14159265f / 180.0f);
        return rotateAxisAngle(worldUp, f, roll);
    }
    
    Vec3 localToWorld(const Vec3& local) const {
        const float deg2rad = 3.14159265f / 180.0f;
        const float yaw   = rotation.y * deg2rad;
        const float pitch = rotation.x * deg2rad;
        const float roll  = rotation.z * deg2rad;

        Vec3 p(local.x * scale.x, local.y * scale.y, local.z * scale.z);

        // Roll
        float cr = cosf(roll),  sr = sinf(roll);
        p = { p.x * cr - p.y * sr,
              p.x * sr + p.y * cr,
              p.z };

        // Pitch
        float cp = cosf(pitch), sp = sinf(pitch);
        p = { p.x,
              p.y * cp - p.z * sp,
              p.y * sp + p.z * cp };

        // Yaw
        float cy = cosf(yaw),   sy = sinf(yaw);
        p = { p.x * cy + p.z * sy,
              p.y,
              -p.x * sy + p.z * cy };

        return { p.x + position.x, p.y + position.y, p.z + position.z };
    }

    Vec3 worldToLocal(const Vec3& worldPoint) const {
        Vec3 p(worldPoint.x - position.x,
               worldPoint.y - position.y,
               worldPoint.z - position.z);

        const float deg2rad = 3.14159265f / 180.0f;
        const float yaw   = rotation.y * deg2rad;
        const float pitch = rotation.x * deg2rad;
        const float roll  = rotation.z * deg2rad;

        // Inverse yaw
        float cy = cosf(yaw),   sy = sinf(yaw);
        p = { p.x * cy - p.z * sy,
              p.y,
              p.x * sy + p.z * cy };

        // Inverse pitch
        float cp = cosf(pitch), sp = sinf(pitch);
        p = { p.x,
              p.y * cp + p.z * sp,
              -p.y * sp + p.z * cp };

        // Inverse roll
        float cr = cosf(roll),  sr = sinf(roll);
        p = { p.x * cr + p.y * sr,
              -p.x * sr + p.y * cr,
              p.z };

        const float sx = (std::fabs(scale.x) > 1e-8f) ? scale.x : 1e-8f;
        const float sy_ = (std::fabs(scale.y) > 1e-8f) ? scale.y : 1e-8f;
        const float sz = (std::fabs(scale.z) > 1e-8f) ? scale.z : 1e-8f;
        return { p.x / sx, p.y / sy_, p.z / sz };
    }
};