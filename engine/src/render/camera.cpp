#include "engine/render/camera.h"
#include "engine/core/transform.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

struct Camera::CameraData {
    glm::mat4 projection;
    glm::mat4 view;
};

namespace {
    glm::mat4 reverseZPerspective(float fovRadians, float aspect, float zNear, float zFar) {
        const float f = 1.0f / tan(fovRadians * 0.5f);

        glm::mat4 proj(0.0f);
        proj[0][0] = f / aspect;
        proj[1][1] = f;
        proj[2][2] = (zFar + zNear) / (zFar - zNear);
        proj[2][3] = -1.0f;
        proj[3][2] = (2.0f * zFar * zNear) / (zFar - zNear);
        return proj;
    }
}

Camera::Camera(float fov, float aspect, float zNear, float zFar) {
    m_data = new CameraData();
    m_data->projection = reverseZPerspective(glm::radians(fov), aspect, zNear, zFar);

    m_fov = fov;
    m_aspect = aspect;
    m_near = zNear;
    m_far = zFar;
}

void* Camera::getViewMatrix(const Transform& transform) const {
    float yaw   = transform.rotation.y;
    float pitch = transform.rotation.x;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(front);

    glm::vec3 pos(transform.position.x, transform.position.y, transform.position.z);
    m_data->view = glm::lookAt(pos, pos + front, glm::vec3(0, 1, 0));
    return &m_data->view;
}

void* Camera::getProjectionMatrix() const {
    return &m_data->projection;
}

Mat4 Camera::getProjectionMatrix(Mat4& out) const {
    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++)
            out[col][row] = m_data->projection[col][row];
    return out;
}

Mat4 Camera::getInvViewMatrix(const Transform& transform) const {
    getViewMatrix(transform);
    glm::mat4 inv = glm::inverse(m_data->view);
    Mat4 out;
    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++)
            out[col][row] = inv[col][row];
    return out;
}

Mat4 Camera::getInvProjMatrix() const {
    glm::mat4 inv = glm::inverse(m_data->projection);
    Mat4 out;
    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++)
            out[col][row] = inv[col][row];
    return out;
}

void Camera::rebuildProjection() {
    m_data->projection = reverseZPerspective(glm::radians(m_fov), m_aspect, m_near, m_far);
}

void Camera::setAspectRatio(float aspect) {
    m_aspect = aspect;
    rebuildProjection();
}

void Camera::setFov(float fov) {
    m_fov = fov;
    rebuildProjection();
}

void Camera::setNear(float nearPlane) {
    m_near = nearPlane;
    rebuildProjection();
}

void Camera::setFar(float farPlane) {
    m_far = farPlane;
    rebuildProjection();
}

Camera::~Camera() {
    delete m_data;
}