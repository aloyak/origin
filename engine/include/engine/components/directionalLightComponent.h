#pragma once

#include "engine/components/component.h"
#include "engine/lighting/directionalLight.h"
#include "engine/core/math.h"

class DirectionalLightComponent : public Component {
public:
    DirectionalLightComponent(Vec3 direction = Vec3(0, -1, 0), 
                             Vec3 color = Vec3(1, 1, 1), 
                             float intensity = 1.0f);

    void setDirection(Vec3 direction) { m_light.setDirection(direction); }
    Vec3 getDirection() const { return m_light.getDirection(); }

    void setColor(Vec3 color) { m_light.setColor(color); }
    Vec3 getColor() const { return m_light.getColor(); }

    void setIntensity(float intensity) { m_light.setIntensity(intensity); }
    float getIntensity() const { return m_light.getIntensity(); }

    const DirectionalLight& getLight() const { return m_light; }
    DirectionalLight& getLight() { return m_light; }

    void serialize(nlohmann::json& j) const override;
    void deserialize(const nlohmann::json& j) override;

private:
    DirectionalLight m_light;
};
