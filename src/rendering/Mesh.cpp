#include "rendering/Mesh.h"
#include "core/Model.h"
#include "core/Node.h"
#include "core/Element.h"
#include <GL/glew.h>

Mesh::Mesh() 
    : m_VAO(0), m_VBO(0), m_EBO(0),
      m_WireVAO(0), m_WireVBO(0), m_WireEBO(0),
      m_NodeVAO(0), m_NodeVBO(0),
      m_IsSetup(false) {
}

Mesh::~Mesh() {
    Clear();
}

void Mesh::BuildFromModel(Model* model) {
    if (!model) return;
    
    Clear();
    
    // Collect node positions
    for (const auto& node : model->GetNodes()) {
        m_NodePositions.push_back(node.position);
    }
    
    // Build mesh from elements
    for (const auto& element : model->GetElements()) {
        std::vector<glm::vec3> positions;
        
        // Get positions for this element
        for (int nodeId : element.nodeIds) {
            Node* node = model->GetNode(nodeId);
            if (node) {
                positions.push_back(node->position);
            }
        }
        
        if (positions.size() >= 3) {
            // Calculate normal for the face
            glm::vec3 normal = CalculateNormal(positions[0], positions[1], positions[2]);
            
            // Add vertices
            unsigned int baseIndex = m_Vertices.size();
            for (const auto& pos : positions) {
                Vertex vertex;
                vertex.position = pos;
                vertex.normal = normal;
                vertex.texCoords = glm::vec2(0.0f, 0.0f);
                m_Vertices.push_back(vertex);
            }
            
            // Add indices for triangulation
            if (element.type == ElementType::SHELL3) {
                m_Indices.push_back(baseIndex);
                m_Indices.push_back(baseIndex + 1);
                m_Indices.push_back(baseIndex + 2);
            } else if (element.type == ElementType::SHELL4) {
                // Triangulate quad
                m_Indices.push_back(baseIndex);
                m_Indices.push_back(baseIndex + 1);
                m_Indices.push_back(baseIndex + 2);
                m_Indices.push_back(baseIndex);
                m_Indices.push_back(baseIndex + 2);
                m_Indices.push_back(baseIndex + 3);
            }
            
            // Add wireframe indices
            for (size_t i = 0; i < positions.size(); ++i) {
                m_WireIndices.push_back(baseIndex + i);
                m_WireIndices.push_back(baseIndex + ((i + 1) % positions.size()));
            }
        }
    }
    
    SetupBuffers();
}

void Mesh::SetupBuffers() {
    if (m_IsSetup) Clear();
    
    // Setup node VAO
    if (!m_NodePositions.empty()) {
        glGenVertexArrays(1, &m_NodeVAO);
        glGenBuffers(1, &m_NodeVBO);
        
        glBindVertexArray(m_NodeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_NodeVBO);
        glBufferData(GL_ARRAY_BUFFER, m_NodePositions.size() * sizeof(glm::vec3),
                     m_NodePositions.data(), GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);
    }
    
    // Setup solid mesh VAO
    if (!m_Vertices.empty() && !m_Indices.empty()) {
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);
        
        glBindVertexArray(m_VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex),
                     m_Vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int),
                     m_Indices.data(), GL_STATIC_DRAW);
        
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                            (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);
        
        // TexCoord attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                            (void*)offsetof(Vertex, texCoords));
        glEnableVertexAttribArray(2);
    }
    
    // Setup wireframe VAO
    if (!m_Vertices.empty() && !m_WireIndices.empty()) {
        glGenVertexArrays(1, &m_WireVAO);
        glGenBuffers(1, &m_WireVBO);
        glGenBuffers(1, &m_WireEBO);
        
        glBindVertexArray(m_WireVAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, m_WireVBO);
        glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex),
                     m_Vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_WireEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_WireIndices.size() * sizeof(unsigned int),
                     m_WireIndices.data(), GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
    }
    
    glBindVertexArray(0);
    m_IsSetup = true;
}

void Mesh::RenderNodes() {
    if (m_NodeVAO) {
        glBindVertexArray(m_NodeVAO);
        glDrawArrays(GL_POINTS, 0, m_NodePositions.size());
        glBindVertexArray(0);
    }
}

void Mesh::RenderWireframe() {
    if (m_WireVAO) {
        glBindVertexArray(m_WireVAO);
        glDrawElements(GL_LINES, m_WireIndices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void Mesh::RenderSolid() {
    if (m_VAO) {
        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

glm::vec3 Mesh::CalculateNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) {
    return glm::normalize(glm::cross(v2 - v1, v3 - v1));
}

void Mesh::Clear() {
    if (m_NodeVAO) glDeleteVertexArrays(1, &m_NodeVAO);
    if (m_NodeVBO) glDeleteBuffers(1, &m_NodeVBO);
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_EBO) glDeleteBuffers(1, &m_EBO);
    if (m_WireVAO) glDeleteVertexArrays(1, &m_WireVAO);
    if (m_WireVBO) glDeleteBuffers(1, &m_WireVBO);
    if (m_WireEBO) glDeleteBuffers(1, &m_WireEBO);
    
    m_Vertices.clear();
    m_Indices.clear();
    m_WireIndices.clear();
    m_NodePositions.clear();
    
    m_IsSetup = false;
}