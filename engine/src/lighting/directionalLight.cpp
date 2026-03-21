#include "engine/lighting/directionalLight.h"

DirectionalLight::DirectionalLight(Vec3 direction, Vec3 color, float intensity)
    : m_direction(direction.normalize()), m_color(color), m_intensity(intensity)
{}
