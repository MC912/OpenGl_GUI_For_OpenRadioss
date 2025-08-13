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