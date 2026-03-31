#include "engine/components/pointLightComponent.h"

#include <nlohmann/json.hpp>

PointLightComponent::PointLightComponent(Vec3 color, float intensity, float radius)
    : m_light(Vec3(0, 0, 0), color, intensity, radius)
{}

std::unique_ptr<Component> PointLightComponent::clone() const {
    auto copy = std::make_unique<PointLightComponent>(m_light.getColor(), m_light.getIntensity(), m_light.getRadius());
    copy->isEnabled = isEnabled;
    return copy;
}

void PointLightComponent::serialize(nlohmann::json& j) const {
    j["type"] = "PointLightComponent";
    j["color"] = {m_light.getColor().x, m_light.getColor().y, m_light.getColor().z};
    j["intensity"] = m_light.getIntensity();
    j["radius"] = m_light.getRadius();
}

void PointLightComponent::deserialize(const nlohmann::json& j) {
    if (j.contains("color")) {
        auto col = j["color"];
        m_light.setColor(Vec3(col[0], col[1], col[2]));
    }
    if (j.contains("intensity")) {
        m_light.setIntensity(j["intensity"]);
    }
    if (j.contains("radius")) {
        m_light.setRadius(j["radius"]);
    }
}
