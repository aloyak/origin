#pragma once

#include "engine/core/math.h"
#include "engine/core/transform.h"

class Camera {
public:
    Camera(float fov, float aspect, float zNear, float zFar);
    ~Camera();

    void* getViewMatrix(const Transform& transform) const;
    void* getProjectionMatrix() const;

    void setAspectRatio(float aspect);

    void setFov(float fov) { m_fov = fov; }
    float getFov() const { return m_fov; }

    void setNear(float near) { m_near = near; }
    float getNear() const { return m_near; }
    
    void setFar(float far) { m_far = far; }
    float getFar() const { return m_far; }
private:
    struct CameraData;
    CameraData* m_data;

    float m_fov, m_aspect, m_near, m_far;
};