#pragma once
#include "engine/math.h"

class Camera {
public: 
    Camera(float fov, float aspect, float zNear, float zFar);
    ~Camera();

    void* getViewMatrix() const;
    void* getProjectionMatrix() const;

    void setPosition(Vec3 pos);

private:
    struct CameraData; 
    CameraData* m_data;
};