#include "engine/lighting/lightingManager.h"
#include "engine/components/entity.h"
#include "engine/components/directionalLightComponent.h"

LightingManager& LightingManager::instance() {
    static LightingManager s_instance;
    return s_instance;
}

void LightingManager::updateLights(const std::vector<Entity*>& entities) {
    m_directionalLights.clear();

    for (Entity* entity : entities) {
        if (!entity) continue;

        auto* dirLightComp = entity->getComponent<DirectionalLightComponent>();
        if (dirLightComp && dirLightComp->isEnabled) {
            m_directionalLights.push_back(dirLightComp->getLight());
        }
    }
}
