#include "engine/components/camera.h"
#include "engine/components/entity.h"
#include <nlohmann/json.hpp>

CameraComponent::CameraComponent(float fov, float aspect, float zNear, float zFar)
    : m_camera(std::make_unique<Camera>(fov, aspect, zNear, zFar))
{
    // Save values for serialization
    m_fov = fov;
    m_aspect = aspect;
    m_near = zNear;
    m_far = zFar;
}

void CameraComponent::serialize(nlohmann::json& j) const {
    j["type"] = "CameraComponent";
    j["fov"] = m_fov;
    j["aspect"] = m_aspect;
    j["near"] = m_near;
    j["far"] = m_far;
}

void CameraComponent::deserialize(const nlohmann::json& j) {
    m_fov = j.value("fov", 60.0f);
    m_aspect = j.value("aspect", 1.0f);
    m_near = j.value("near", 0.1f);
    m_far = j.value("far", 1000.0f);
    
    m_camera = std::make_unique<Camera>(m_fov, m_aspect, m_near, m_far);
}