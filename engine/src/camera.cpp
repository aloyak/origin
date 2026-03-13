#include "engine/camera.h"
#include "engine/transform.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

struct Camera::CameraData {
    glm::mat4 projection;
    glm::mat4 view;
};

Camera::Camera(float fov, float aspect, float zNear, float zFar) {
    m_data = new CameraData();
    m_data->projection = glm::perspective(glm::radians(fov), aspect, zNear, zFar);
}

void* Camera::getViewMatrix(const Transform& transform) const {
    float yaw   = transform.rotation.y;
    float pitch = glm::clamp(transform.rotation.x, -89.0f, 89.0f);

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

void Camera::setAspectRatio(float aspect) {
    float fov = glm::degrees(2.0f * atan(1.0f / m_data->projection[1][1]));
    float zNear = m_data->projection[3][2] / (m_data->projection[2][2] - 1.0f);
    float zFar = m_data->projection[3][2] / (m_data->projection[2][2] + 1.0f);
    m_data->projection = glm::perspective(glm::radians(fov), aspect, zNear, zFar);
}

Camera::~Camera() {
    delete m_data;
}