#include "core/Model.h"
#include <algorithm>
#include <limits>

Model::Model() 
    : m_MinBounds(0.0f), m_MaxBounds(0.0f) {
}

void Model::AddNode(const Node& node) {
    m_NodeIdToIndex[node.id] = m_Nodes.size();
    m_Nodes.push_back(node);
}

void Model::RemoveNode(int nodeId) {
    auto it = m_NodeIdToIndex.find(nodeId);
    if (it != m_NodeIdToIndex.end()) {
        size_t index = it->second;
        m_Nodes.erase(m_Nodes.begin() + index);
        m_NodeIdToIndex.erase(it);
        
        // Update indices
        for (auto& pair : m_NodeIdToIndex) {
            if (pair.second > index) {
                pair.second--;
            }
        }
    }
}

Node* Model::GetNode(int nodeId) {
    auto it = m_NodeIdToIndex.find(nodeId);
    if (it != m_NodeIdToIndex.end()) {
        return &m_Nodes[it->second];
    }
    return nullptr;
}

void Model::AddElement(const Element& element) {
    m_ElementIdToIndex[element.id] = m_Elements.size();
    m_Elements.push_back(element);
}

void Model::RemoveElement(int elementId) {
    auto it = m_ElementIdToIndex.find(elementId);
    if (it != m_ElementIdToIndex.end()) {
        size_t index = it->second;
        m_Elements.erase(m_Elements.begin() + index);
        m_ElementIdToIndex.erase(it);
        
        // Update indices
        for (auto& pair : m_ElementIdToIndex) {
            if (pair.second > index) {
                pair.second--;
            }
        }
    }
}

Element* Model::GetElement(int elementId) {
    auto it = m_ElementIdToIndex.find(elementId);
    if (it != m_ElementIdToIndex.end()) {
        return &m_Elements[it->second];
    }
    return nullptr;
}

void Model::AddMaterial(const Material& material) {
    m_MaterialIdToIndex[material.id] = m_Materials.size();
    m_Materials.push_back(material);
}

Material* Model::GetMaterial(int materialId) {
    auto it = m_MaterialIdToIndex.find(materialId);
    if (it != m_MaterialIdToIndex.end()) {
        return &m_Materials[it->second];
    }
    return nullptr;
}

void Model::CalculateBounds() {
    if (m_Nodes.empty()) {
        m_MinBounds = m_MaxBounds = glm::vec3(0.0f);
        return;
    }
    
    m_MinBounds = glm::vec3(std::numeric_limits<float>::max());
    m_MaxBounds = glm::vec3(std::numeric_limits<float>::lowest());
    
    for (const auto& node : m_Nodes) {
        m_MinBounds = glm::min(m_MinBounds, node.position);
        m_MaxBounds = glm::max(m_MaxBounds, node.position);
    }
}

float Model::GetBoundingRadius() const {
    glm::vec3 center = GetCenter();
    float maxDist = 0.0f;
    
    for (const auto& node : m_Nodes) {
        float dist = glm::length(node.position - center);
        maxDist = std::max(maxDist, dist);
    }
    
    return maxDist;
}

void Model::Clear() {
    m_Nodes.clear();
    m_Elements.clear();
    m_Materials.clear();
    m_NodeIdToIndex.clear();
    m_ElementIdToIndex.clear();
    m_MaterialIdToIndex.clear();
    m_MinBounds = m_MaxBounds = glm::vec3(0.0f);
}
