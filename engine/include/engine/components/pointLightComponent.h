#pragma once

#include "engine/components/component.h"
#include "engine/lighting/pointLight.h"
#include "engine/core/math.h"

class PointLightComponent : public Component {
public:
    PointLightComponent(Vec3 color = Vec3(1, 1, 1), float intensity = 0.75f, float radius = 150.0f);

    void setColor(Vec3 color) { m_light.setColor(color); }
    Vec3 getColor() const { return m_light.getColor(); }

    void setIntensity(float intensity) { m_light.setIntensity(intensity); }
    float getIntensity() const { return m_light.getIntensity(); }

    void setRadius(float radius) { m_light.setRadius(radius); }
    float getRadius() const { return m_light.getRadius(); }

    const PointLight& getLight() const { return m_light; }
    PointLight& getLight() { return m_light; }

    std::unique_ptr<Component> clone() const override;

    void serialize(nlohmann::json& j) const override;
    void deserialize(const nlohmann::json& j) override;

private:
    PointLight m_light;
};
