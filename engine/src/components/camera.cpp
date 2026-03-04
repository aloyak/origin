#include "engine/components/camera.h"
#include "engine/components/entity.h"

CameraComponent::CameraComponent(float fov, float aspect, float zNear, float zFar)
    : m_camera(std::make_unique<Camera>(fov, aspect, zNear, zFar))
{
}