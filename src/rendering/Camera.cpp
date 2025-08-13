#include "rendering/Camera.h"
#include "core/Model.h"
#include <algorithm>

Camera::Camera(int windowWidth, int windowHeight)
    : m_Position(0.0f, 0.0f, 5.0f),
      m_Target(0.0f, 0.0f, 0.0f),
      m_WorldUp(0.0f, 1.0f, 0.0f),
      m_Yaw(-90.0f),
      m_Pitch(0.0f),
      m_Distance(5.0f),
      m_Fov(45.0f),
      m_AspectRatio(float(windowWidth) / float(windowHeight)),
      m_NearPlane(0.1f),
      m_FarPlane(1000.0f),
      m_MovementSpeed(2.5f),
      m_MouseSensitivity(0.1f),
      m_ZoomSpeed(2.0f) {
    UpdateCameraVectors();
}

void Camera::Update(float deltaTime) {
    // Any per-frame camera updates
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset) {
    xoffset *= m_MouseSensitivity;
    yoffset *= m_MouseSensitivity;
    
    m_Yaw += xoffset;
    m_Pitch += yoffset;
    
    // Constrain pitch
    if (m_Pitch > 89.0f) m_Pitch = 89.0f;
    if (m_Pitch < -89.0f) m_Pitch = -89.0f;
    
    UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
    m_Distance -= yoffset * m_ZoomSpeed;
    m_Distance = std::max(0.1f, std::min(m_Distance, 500.0f));
    UpdateCameraVectors();
}

void Camera::FitToModel(Model* model) {
    if (!model || model->GetNodeCount() == 0) return;
    
    model->CalculateBounds();
    m_Target = model->GetCenter();
    m_Distance = model->GetBoundingRadius() * 2.5f;
    
    UpdateCameraVectors();
}

void Camera::Reset() {
    m_Position = glm::vec3(0.0f, 0.0f, 5.0f);
    m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
    m_Yaw = -90.0f;
    m_Pitch = 0.0f;
    m_Distance = 5.0f;
    
    UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(m_Position, m_Target, m_Up);
}

glm::mat4 Camera::GetProjectionMatrix() const {
    return glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_NearPlane, m_FarPlane);
}

void Camera::UpdateCameraVectors() {
    // Calculate position from spherical coordinates
    float x = m_Distance * cos(glm::radians(m_Pitch)) * cos(glm::radians(m_Yaw));
    float y = m_Distance * sin(glm::radians(m_Pitch));
    float z = m_Distance * cos(glm::radians(m_Pitch)) * sin(glm::radians(m_Yaw));
    
    m_Position = m_Target + glm::vec3(x, y, z);
    
    // Calculate camera vectors
    glm::vec3 front = glm::normalize(m_Target - m_Position);
    m_Right = glm::normalize(glm::cross(front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, front));
}