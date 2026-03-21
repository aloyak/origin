#pragma once

#include "../lighting/directionalLight.h"
#include <vector>

class Entity;

class LightingManager {
public:
    static LightingManager& instance();

    // Collect all active DirectionalLightComponents from scene
    void updateLights(const std::vector<Entity*>& entities);

    // Get current directional lights for rendering
    const std::vector<DirectionalLight>& getDirectionalLights() const { return m_directionalLights; }

private:
    LightingManager() = default;

    std::vector<DirectionalLight> m_directionalLights;
};
