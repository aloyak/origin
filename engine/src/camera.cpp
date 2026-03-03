#include "engine/camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera::CameraData {
    glm::vec3 position;
    glm::mat4 projection;
    glm::mat4 view;
};

Camera::Camera(float fov, float aspect, float zNear, float zFar) {
    m_data = new CameraData();
    m_data->position = glm::vec3(0.0f, 0.0f, 3.0f);
    m_data->projection = glm::perspective(glm::radians(fov), aspect, zNear, zFar);
}

void* Camera::getViewMatrix() const {
    m_data->view = glm::lookAt(m_data->position, m_data->position + glm::vec3(0,0,-1), glm::vec3(0,1,0));
    return &m_data->view; 
}

void* Camera::getProjectionMatrix() const {
    return &m_data->projection;
}

void Camera::setPosition(Vec3 pos) {
    m_data->position = glm::vec3(pos.x, pos.y, pos.z);
}

Camera::~Camera() {
    delete m_data;
}