#include "engine/lighting/lightingManager.h"
#include "engine/components/entity.h"
#include "engine/components/directionalLightComponent.h"
#include "engine/components/pointLightComponent.h"

LightingManager& LightingManager::instance() {
    static LightingManager s_instance;
    return s_instance;
}

void LightingManager::updateLights(const std::vector<Entity*>& entities) {
    m_directionalLights.clear();
    m_pointLights.clear();

    for (Entity* entity : entities) {
        if (!entity) continue;

        auto* dirLightComp = entity->getComponent<DirectionalLightComponent>();
        if (dirLightComp && dirLightComp->isEnabled) {
            m_directionalLights.push_back(dirLightComp->getLight());
        }

        auto* pointLightComp = entity->getComponent<PointLightComponent>();
        if (pointLightComp && pointLightComp->isEnabled) {
            PointLight light = pointLightComp->getLight();
            light.setPosition(entity->transform.position);
            m_pointLights.push_back(light);
        }
    }
}
