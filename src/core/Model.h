// src/core/Model.h
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

// src/core/Model.cpp
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

// src/core/Material.h
#pragma once
#include <string>

enum class MaterialType {
    ELASTIC,
    PLASTIC,
    JOHNSON_COOK,
    COMPOSITE,
    HYPERELASTIC
};

struct Material {
    int id;
    std::string name;
    MaterialType type;
    
    // Common properties
    float density;
    float youngModulus;
    float poissonRatio;
    
    // Plastic properties
    float yieldStress;
    float tangentModulus;
    
    // Johnson-Cook parameters
    float jc_A, jc_B, jc_n, jc_C, jc_m;
    float jc_D1, jc_D2, jc_D3, jc_D4, jc_D5;
    
    Material() : id(0), type(MaterialType::ELASTIC),
                 density(0.0f), youngModulus(0.0f), poissonRatio(0.0f),
                 yieldStress(0.0f), tangentModulus(0.0f),
                 jc_A(0.0f), jc_B(0.0f), jc_n(0.0f), jc_C(0.0f), jc_m(0.0f),
                 jc_D1(0.0f), jc_D2(0.0f), jc_D3(0.0f), jc_D4(0.0f), jc_D5(0.0f) {}
};

// src/io/FileManager.h
#pragma once
#include <string>
#include <memory>

class Model;

class FileManager {
public:
    FileManager(Model* model);
    ~FileManager() = default;
    
    // RAD file operations
    bool LoadRadFile(const std::string& filepath);
    bool SaveRadFile(const std::string& filepath);
    
    // Export functions
    bool ExportToSTL(const std::string& filepath);
    bool ExportToVTK(const std::string& filepath);
    
    // Import functions
    bool ImportFromNastran(const std::string& filepath);
    bool ImportFromAbaqus(const std::string& filepath);
    
    std::string GetCurrentFile() const { return m_CurrentFile; }
    
private:
    Model* m_Model;
    std::string m_CurrentFile;
};

// src/io/FileManager.cpp
#include "io/FileManager.h"
#include "io/RadFileReader.h"
#include "io/RadFileWriter.h"
#include "core/Model.h"
#include "utils/Logger.h"

FileManager::FileManager(Model* model) 
    : m_Model(model) {
}

bool FileManager::LoadRadFile(const std::string& filepath) {
    RadFileReader reader(m_Model);
    
    if (reader.Read(filepath)) {
        m_CurrentFile = filepath;
        m_Model->CalculateBounds();
        return true;
    }
    
    return false;
}

bool FileManager::SaveRadFile(const std::string& filepath) {
    RadFileWriter writer(m_Model);
    
    if (writer.Write(filepath)) {
        m_CurrentFile = filepath;
        return true;
    }
    
    return false;
}

bool FileManager::ExportToSTL(const std::string& filepath) {
    // Implementation for STL export
    LOG_INFO("Exporting to STL: {}", filepath);
    // TODO: Implement STL export
    return false;
}

bool FileManager::ExportToVTK(const std::string& filepath) {
    // Implementation for VTK export
    LOG_INFO("Exporting to VTK: {}", filepath);
    // TODO: Implement VTK export
    return false;
}

// src/io/RadFileReader.h
#pragma once
#include <string>
#include <fstream>

class Model;

class RadFileReader {
public:
    RadFileReader(Model* model);
    ~RadFileReader() = default;
    
    bool Read(const std::string& filepath);
    
private:
    bool ParseLine(const std::string& line);
    bool ParseNodeSection();
    bool ParseElementSection(const std::string& elementType);
    bool ParseMaterialSection();
    
    void ParseNode(const std::string& line);
    void ParseShell3(const std::string& line);
    void ParseShell4(const std::string& line);
    void ParseTetra4(const std::string& line);
    void ParseHexa8(const std::string& line);
    
    std::string Trim(const std::string& str);
    bool IsCommentLine(const std::string& line);
    
private:
    Model* m_Model;
    std::ifstream m_File;
    std::string m_CurrentSection;
    int m_LineNumber;
};

// src/io/RadFileReader.cpp
#include "io/RadFileReader.h"
#include "core/Model.h"
#include "core/Node.h"
#include "core/Element.h"
#include "utils/Logger.h"
#include <sstream>
#include <algorithm>

RadFileReader::RadFileReader(Model* model) 
    : m_Model(model), m_LineNumber(0) {
}

bool RadFileReader::Read(const std::string& filepath) {
    m_File.open(filepath);
    if (!m_File.is_open()) {
        LOG_ERROR("Cannot open file: {}", filepath);
        return false;
    }
    
    LOG_INFO("Reading RAD file: {}", filepath);
    m_Model->Clear();
    
    std::string line;
    while (std::getline(m_File, line)) {
        m_LineNumber++;
        
        if (!ParseLine(line)) {
            LOG_WARN("Error parsing line {}: {}", m_LineNumber, line);
        }
    }
    
    m_File.close();
    
    LOG_INFO("Loaded {} nodes, {} elements", 
             m_Model->GetNodeCount(), m_Model->GetElementCount());
    
    return true;
}

bool RadFileReader::ParseLine(const std::string& line) {
    if (IsCommentLine(line)) {
        return true;
    }
    
    std::string trimmed = Trim(line);
    if (trimmed.empty()) {
        return true;
    }
    
    // Check for section headers
    if (trimmed[0] == '/') {
        if (trimmed.find("/NODE") == 0) {
            m_CurrentSection = "NODE";
        } else if (trimmed.find("/SH3N") == 0) {
            m_CurrentSection = "SH3N";
        } else if (trimmed.find("/SHELL") == 0) {
            m_CurrentSection = "SHELL";
        } else if (trimmed.find("/TETRA") == 0) {
            m_CurrentSection = "TETRA";
        } else if (trimmed.find("/BRICK") == 0) {
            m_CurrentSection = "BRICK";
        } else if (trimmed.find("/MAT") == 0) {
            m_CurrentSection = "MATERIAL";
        } else if (trimmed.find("/END") == 0) {
            m_CurrentSection = "";
            return true;
        }
        return true;
    }
    
    // Parse data based on current section
    if (m_CurrentSection == "NODE") {
        ParseNode(trimmed);
    } else if (m_CurrentSection == "SH3N") {
        ParseShell3(trimmed);
    } else if (m_CurrentSection == "SHELL") {
        ParseShell4(trimmed);
    } else if (m_CurrentSection == "TETRA") {
        ParseTetra4(trimmed);
    } else if (m_CurrentSection == "BRICK") {
        ParseHexa8(trimmed);
    }
    
    return true;
}

void RadFileReader::ParseNode(const std::string& line) {
    std::istringstream iss(line);
    int id;
    float x, y, z;
    
    if (iss >> id >> x >> y >> z) {
        Node node(id, glm::vec3(x, y, z));
        m_Model->AddNode(node);
    }
}

void RadFileReader::ParseShell3(const std::string& line) {
    std::istringstream iss(line);
    Element element;
    element.type = ElementType::SHELL3;
    
    if (iss >> element.id) {
        int nodeId;
        while (iss >> nodeId) {
            element.nodeIds.push_back(nodeId);
        }
        
        if (element.nodeIds.size() >= 3) {
            m_Model->AddElement(element);
        }
    }
}

void RadFileReader::ParseShell4(const std::string& line) {
    std::istringstream iss(line);
    Element element;
    element.type = ElementType::SHELL4;
    
    if (iss >> element.id) {
        int nodeId;
        while (iss >> nodeId) {
            element.nodeIds.push_back(nodeId);
        }
        
        if (element.nodeIds.size() >= 4) {
            m_Model->AddElement(element);
        }
    }
}

std::string RadFileReader::Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

bool RadFileReader::IsCommentLine(const std::string& line) {
    std::string trimmed = Trim(line);
    return trimmed.empty() || trimmed[0] == '#' || trimmed[0] == '$' ||
           trimmed.substr(0, 2) == "//";
}