#pragma once
#include <memory>
#include <glm/glm.hpp>

struct GLFWwindow;
class Model;
class Camera;
class Shader;
class Mesh;

struct RenderSettings {
    bool showNodes = true;
    bool showWireframe = true;
    bool showSolid = false;
    bool showNormals = false;
    bool enableLighting = true;
    
    glm::vec3 backgroundColor = glm::vec3(0.05f, 0.05f, 0.15f);
    glm::vec3 nodeColor = glm::vec3(1.0f, 0.3f, 0.3f);
    glm::vec3 wireframeColor = glm::vec3(0.9f, 0.9f, 0.9f);
    glm::vec3 solidColor = glm::vec3(0.6f, 0.8f, 1.0f);
    
    float nodeSize = 3.0f;
    float lineWidth = 1.0f;
};

class Renderer {
public:
    Renderer(GLFWwindow* window);
    ~Renderer();
    
    void Initialize();
    void Shutdown();
    
    void BeginFrame();
    void EndFrame();
    
    void Update(float deltaTime);
    void RenderModel(Model* model);
    void UpdateMesh(Model* model);
    
    // Settings
    RenderSettings& GetSettings() { return m_Settings; }
    void SetSettings(const RenderSettings& settings) { m_Settings = settings; }
    
    // Camera
    Camera* GetCamera() { return m_Camera.get(); }
    
    GLFWwindow* GetWindow() { return m_Window; }
    
private:
    void SetupShaders();
    void RenderNodes(Model* model);
    void RenderWireframe(Model* model);
    void RenderSolid(Model* model);
    
private:
    GLFWwindow* m_Window;
    std::unique_ptr<Camera> m_Camera;
    std::unique_ptr<Shader> m_BasicShader;
    std::unique_ptr<Shader> m_PhongShader;
    std::unique_ptr<Mesh> m_Mesh;
    
    RenderSettings m_Settings;
    
    unsigned int m_NodeVAO, m_NodeVBO;
    unsigned int m_WireVAO, m_WireVBO, m_WireEBO;
    unsigned int m_SolidVAO, m_SolidVBO, m_SolidEBO;
};