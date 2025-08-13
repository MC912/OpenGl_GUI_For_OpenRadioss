#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Model;
struct GLFWwindow;

class Camera {
public:
    Camera(int windowWidth, int windowHeight);
    ~Camera() = default;
    
    void Update(float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset);
    void ProcessMouseScroll(float yoffset);
    
    void FitToModel(Model* model);
    void Reset();
    
    // Getters
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
    glm::vec3 GetPosition() const { return m_Position; }
    glm::vec3 GetTarget() const { return m_Target; }
    
    // Setters
    void SetPosition(const glm::vec3& position) { m_Position = position; }
    void SetTarget(const glm::vec3& target) { m_Target = target; }
    void SetAspectRatio(float aspect) { m_AspectRatio = aspect; }
    
private:
    void UpdateCameraVectors();
    
private:
    glm::vec3 m_Position;
    glm::vec3 m_Target;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;
    
    float m_Yaw;
    float m_Pitch;
    float m_Distance;
    
    float m_Fov;
    float m_AspectRatio;
    float m_NearPlane;
    float m_FarPlane;
    
    float m_MovementSpeed;
    float m_MouseSensitivity;
    float m_ZoomSpeed;
};