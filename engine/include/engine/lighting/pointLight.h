#pragma once

#include "engine/core/math.h"

class PointLight {
public:
    PointLight(Vec3 position = Vec3(0, 0, 0), Vec3 color = Vec3(1, 1, 1), float intensity = 1.0f, float radius = 300.0f);

    void setPosition(Vec3 position) { m_position = position; }
    Vec3 getPosition() const { return m_position; }

    void setColor(Vec3 color) { m_color = color; }
    Vec3 getColor() const { return m_color; }

    void setIntensity(float intensity) { m_intensity = intensity; }
    float getIntensity() const { return m_intensity; }

    void setRadius(float radius) { m_radius = radius; }
    float getRadius() const { return m_radius; }

private:
    Vec3 m_position;
    Vec3 m_color;
    float m_intensity;
    float m_radius;
};
