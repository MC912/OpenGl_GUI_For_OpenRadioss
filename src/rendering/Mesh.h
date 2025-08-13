#pragma once
#include <vector>
#include <glm/glm.hpp>

class Model;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

class Mesh {
public:
    Mesh();
    ~Mesh();
    
    void BuildFromModel(Model* model);
    void Clear();
    
    void RenderNodes();
    void RenderWireframe();
    void RenderSolid();
    
private:
    void SetupBuffers();
    glm::vec3 CalculateNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);
    
private:
    std::vector<Vertex> m_Vertices;
    std::vector<unsigned int> m_Indices;
    std::vector<unsigned int> m_WireIndices;
    std::vector<glm::vec3> m_NodePositions;
    
    unsigned int m_VAO, m_VBO, m_EBO;
    unsigned int m_WireVAO, m_WireVBO, m_WireEBO;
    unsigned int m_NodeVAO, m_NodeVBO;
    
    bool m_IsSetup;
};
