#include "engine/components/directionalLightComponent.h"
#include <nlohmann/json.hpp>

DirectionalLightComponent::DirectionalLightComponent(Vec3 direction, Vec3 color, float intensity)
    : m_light(direction, color, intensity)
{}

void DirectionalLightComponent::serialize(nlohmann::json& j) const {
    j["type"] = "DirectionalLightComponent";
    j["direction"] = {m_light.getDirection().x, m_light.getDirection().y, m_light.getDirection().z};
    j["color"] = {m_light.getColor().x, m_light.getColor().y, m_light.getColor().z};
    j["intensity"] = m_light.getIntensity();
}

void DirectionalLightComponent::deserialize(const nlohmann::json& j) {
    if (j.contains("direction")) {
        auto dir = j["direction"];
        m_light.setDirection(Vec3(dir[0], dir[1], dir[2]));
    }
    if (j.contains("color")) {
        auto col = j["color"];
        m_light.setColor(Vec3(col[0], col[1], col[2]));
    }
    if (j.contains("intensity")) {
        m_light.setIntensity(j["intensity"]);
    }
}
