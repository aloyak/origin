#include "engine/lighting/pointLight.h"

PointLight::PointLight(Vec3 position, Vec3 color, float intensity, float radius)
    : m_position(position), m_color(color), m_intensity(intensity), m_radius(radius)
{}
