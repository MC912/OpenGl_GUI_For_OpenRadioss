#include "rendering/Renderer.h"
#include "rendering/Camera.h"
#include "rendering/Shader.h"
#include "rendering/Mesh.h"
#include "core/Model.h"
#include "utils/Logger.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Renderer::Renderer(GLFWwindow* window) 
    : m_Window(window), m_NodeVAO(0), m_NodeVBO(0),
      m_WireVAO(0), m_WireVBO(0), m_WireEBO(0),
      m_SolidVAO(0), m_SolidVBO(0), m_SolidEBO(0) {
    Initialize();
}

Renderer::~Renderer() {
    Shutdown();
}

void Renderer::Initialize() {
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error("Failed to initialize GLEW");
    }
    
    // Setup OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    // Create camera
    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    m_Camera = std::make_unique<Camera>(width, height);
    
    // Setup shaders
    SetupShaders();
    
    // Create mesh object
    m_Mesh = std::make_unique<Mesh>();
    
    LOG_INFO("Renderer initialized");
}

void Renderer::SetupShaders() {
    // Basic shader for wireframe and points
    m_BasicShader = std::make_unique<Shader>(
        "shaders/basic.vert", "shaders/basic.frag");
    
    // Phong shader for solid rendering
    m_PhongShader = std::make_unique<Shader>(
        "shaders/phong.vert", "shaders/phong.frag");
}

void Renderer::BeginFrame() {
    glClearColor(m_Settings.backgroundColor.r, 
                 m_Settings.backgroundColor.g,
                 m_Settings.backgroundColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndFrame() {
    // Nothing specific needed here
}

void Renderer::Update(float deltaTime) {
    m_Camera->Update(deltaTime);
}

void Renderer::RenderModel(Model* model) {
    if (!model || model->GetNodeCount() == 0) {
        return;
    }
    
    if (m_Settings.showSolid) {
        RenderSolid(model);
    }
    
    if (m_Settings.showWireframe) {
        RenderWireframe(model);
    }
    
    if (m_Settings.showNodes) {
        RenderNodes(model);
    }
}

void Renderer::UpdateMesh(Model* model) {
    if (!model) return;
    
    m_Mesh->BuildFromModel(model);
    
    // Update camera to fit model
    m_Camera->FitToModel(model);
}

void Renderer::RenderNodes(Model* model) {
    m_BasicShader->Use();
    m_BasicShader->SetMat4("view", m_Camera->GetViewMatrix());
    m_BasicShader->SetMat4("projection", m_Camera->GetProjectionMatrix());
    m_BasicShader->SetMat4("model", glm::mat4(1.0f));
    m_BasicShader->SetVec3("color", m_Settings.nodeColor);
    
    glPointSize(m_Settings.nodeSize);
    m_Mesh->RenderNodes();
}

void Renderer::RenderWireframe(Model* model) {
    m_BasicShader->Use();
    m_BasicShader->SetMat4("view", m_Camera->GetViewMatrix());
    m_BasicShader->SetMat4("projection", m_Camera->GetProjectionMatrix());
    m_BasicShader->SetMat4("model", glm::mat4(1.0f));
    m_BasicShader->SetVec3("color", m_Settings.wireframeColor);
    
    glLineWidth(m_Settings.lineWidth);
    m_Mesh->RenderWireframe();
}

void Renderer::RenderSolid(Model* model) {
    m_PhongShader->Use();
    m_PhongShader->SetMat4("view", m_Camera->GetViewMatrix());
    m_PhongShader->SetMat4("projection", m_Camera->GetProjectionMatrix());
    m_PhongShader->SetMat4("model", glm::mat4(1.0f));
    
    // Lighting
    m_PhongShader->SetVec3("lightPos", m_Camera->GetPosition());
    m_PhongShader->SetVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    m_PhongShader->SetVec3("objectColor", m_Settings.solidColor);
    m_PhongShader->SetVec3("viewPos", m_Camera->GetPosition());
    
    m_Mesh->RenderSolid();
}

void Renderer::Shutdown() {
    // Cleanup OpenGL resources
    if (m_NodeVAO) glDeleteVertexArrays(1, &m_NodeVAO);
    if (m_NodeVBO) glDeleteBuffers(1, &m_NodeVBO);
    if (m_WireVAO) glDeleteVertexArrays(1, &m_WireVAO);
    if (m_WireVBO) glDeleteBuffers(1, &m_WireVBO);
    if (m_WireEBO) glDeleteBuffers(1, &m_WireEBO);
    if (m_SolidVAO) glDeleteVertexArrays(1, &m_SolidVAO);
    if (m_SolidVBO) glDeleteBuffers(1, &m_SolidVBO);
    if (m_SolidEBO) glDeleteBuffers(1, &m_SolidEBO);
}
