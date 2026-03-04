#pragma once

#include "engine/components/component.h"
#include "engine/camera.h"

#include <memory>

class CameraComponent : public Component {
public:
    CameraComponent(float fov, float aspect, float zNear, float zFar);

    Camera& getCamera() { return *m_camera; }

private:
    std::unique_ptr<Camera> m_camera;
};