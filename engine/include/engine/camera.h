#pragma once

#include "engine/math.h"
#include "engine/transform.h"

class Camera {
public:
    Camera(float fov, float aspect, float zNear, float zFar);
    ~Camera();

    void* getViewMatrix(const Transform& transform) const;
    void* getProjectionMatrix() const;

    void setAspectRatio(float aspect);

    void setFov(float fov) { m_fov = fov; }
    float getFov() const { return m_fov; }
private:
    struct CameraData;
    CameraData* m_data;

    float m_fov, m_aspect, m_near, m_far;
};