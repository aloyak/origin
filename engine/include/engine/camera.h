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
private:
    struct CameraData;
    CameraData* m_data;
};