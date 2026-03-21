#pragma once

#include "engine/core/math.h"

class DirectionalLight {
public:
    DirectionalLight(Vec3 direction = Vec3(0, -1, 0), Vec3 color = Vec3(1, 1, 1), float intensity = 1.0f);

    void setDirection(Vec3 direction) { m_direction = direction.normalize(); }
    Vec3 getDirection() const { return m_direction; }

    void setColor(Vec3 color) { m_color = color; }
    Vec3 getColor() const { return m_color; }

    void setIntensity(float intensity) { m_intensity = intensity; }
    float getIntensity() const { return m_intensity; }

private:
    Vec3 m_direction;  // Normalized direction vector pointing FROM light TO scene
    Vec3 m_color;
    float m_intensity;
};
