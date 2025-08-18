#pragma once
#include "Node.h"
#include "Element.h"
#include "Material.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

class Model {
public:
    Model();
    ~Model() = default;
    
    // Node operations
    void AddNode(const Node& node);
    void RemoveNode(int nodeId);
    Node* GetNode(int nodeId);
    const std::vector<Node>& GetNodes() const { return m_Nodes; }
    
    // Element operations
    void AddElement(const Element& element);
    void RemoveElement(int elementId);
    Element* GetElement(int elementId);
    const std::vector<Element>& GetElements() const { return m_Elements; }
    
    // Material operations
    void AddMaterial(const Material& material);
    Material* GetMaterial(int materialId);
    const std::vector<Material>& GetMaterials() const { return m_Materials; }  // THIS WAS MISSING
    
    // Model properties
    void CalculateBounds();
    glm::vec3 GetMinBounds() const { return m_MinBounds; }
    glm::vec3 GetMaxBounds() const { return m_MaxBounds; }
    glm::vec3 GetCenter() const { return (m_MinBounds + m_MaxBounds) * 0.5f; }
    float GetBoundingRadius() const;
    
    // Clear model
    void Clear();
    
    // Statistics
    size_t GetNodeCount() const { return m_Nodes.size(); }
    size_t GetElementCount() const { return m_Elements.size(); }
    size_t GetMaterialCount() const { return m_Materials.size(); }
    
private:
    std::vector<Node> m_Nodes;
    std::vector<Element> m_Elements;
    std::vector<Material> m_Materials;
    
    std::unordered_map<int, size_t> m_NodeIdToIndex;
    std::unordered_map<int, size_t> m_ElementIdToIndex;
    std::unordered_map<int, size_t> m_MaterialIdToIndex;
    
    glm::vec3 m_MinBounds;
    glm::vec3 m_MaxBounds;
};