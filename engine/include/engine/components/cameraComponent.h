#pragma once

#include "engine/components/entity.h"
#include "engine/components/component.h"
#include "engine/render/camera.h"

#include <memory>

class CameraComponent : public Component {
public:
    CameraComponent(float fov, float aspect, float zNear, float zFar);

    Camera& getCamera() { return *m_camera; }

    std::unique_ptr<Component> clone() const override;

    void serialize(nlohmann::json& j) const override;
    void deserialize(const nlohmann::json& j) override;

    void lookAt(Entity& target);
private:
    std::unique_ptr<Camera> m_camera;
    
    float m_fov, m_aspect, m_near, m_far;
};