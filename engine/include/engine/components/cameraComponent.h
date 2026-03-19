#pragma once

#include "engine/components/component.h"
#include "engine/camera.h"

#include <memory>

class CameraComponent : public Component {
public:
    CameraComponent(float fov, float aspect, float zNear, float zFar);

    Camera& getCamera() { return *m_camera; }

    void serialize(nlohmann::json& j) const override;
    void deserialize(const nlohmann::json& j) override;

private:
    std::unique_ptr<Camera> m_camera;
    
    float m_fov, m_aspect, m_near, m_far;
};