#pragma once

#include "engine/core/math.h"
#include "engine/core/transform.h"

class Camera {
public:
    Camera(float fov, float aspect, float zNear, float zFar);
    ~Camera();

    void* getViewMatrix(const Transform& transform) const;
    
    void* getProjectionMatrix() const;
    Mat4  getProjectionMatrix(Mat4& out) const;

    Mat4  getInvViewMatrix(const Transform& transform) const;
    Mat4  getInvProjMatrix() const;

    void setAspectRatio(float aspect);

    void setFov(float fov);
    float getFov() const { return m_fov; }

    void setNear(float nearPlane);
    float getNear() const { return m_near; }
    
    void setFar(float farPlane);
    float getFar() const { return m_far; }

    void setAspect(float aspect) { setAspectRatio(aspect); }
    float getAspectRatio() const { return m_aspect; }
private:
    struct CameraData;
    CameraData* m_data;

    void rebuildProjection();

    float m_fov, m_aspect, m_near, m_far;
};