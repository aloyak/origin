#pragma once

#include "engine/lighting/directionalLight.h"
#include "engine/lighting/pointLight.h"
#include <vector>

class Entity;

class LightingManager {
public:
    static LightingManager& instance();

    // Collect all active DirectionalLightComponents from scene
    void updateLights(const std::vector<Entity*>& entities);

    // Get current directional lights for rendering
    const std::vector<DirectionalLight>& getDirectionalLights() const { return m_directionalLights; }
    const std::vector<PointLight>& getPointLights() const { return m_pointLights; }

private:
    LightingManager() = default;

    std::vector<DirectionalLight> m_directionalLights;
    std::vector<PointLight> m_pointLights;
};
