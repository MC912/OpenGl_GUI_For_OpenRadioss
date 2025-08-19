#include "radfilereader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace OpenRadiossGUI {

RadFileReader::RadFileReader() : isValid_(false) {
}

RadFileReader::~RadFileReader() {
    clear();
}

bool RadFileReader::loadFile(const std::string& filename) {
    clear();
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        lastError_ = "Cannot open file: " + filename;
        return false;
    }
    
    // Simple RAD file parsing (basic implementation)
    std::string line;
    bool inNodes = false;
    bool inElements = false;
    
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#' || line[0] == 'C') {
            continue;
        }
        
        // Check for section headers
        if (line.find("/NODE") != std::string::npos) {
            inNodes = true;
            inElements = false;
            continue;
        }
        
        if (line.find("/SHELL") != std::string::npos || 
            line.find("/BRICK") != std::string::npos ||
            line.find("/TRIA") != std::string::npos) {
            inNodes = false;
            inElements = true;
            continue;
        }
        
        if (line.find("/TITLE") != std::string::npos) {
            std::getline(file, title_);
            continue;
        }
        
        // Parse nodes
        if (inNodes) {
            std::istringstream iss(line);
            int id;
            float x, y, z;
            if (iss >> id >> x >> y >> z) {
                nodes_.emplace_back(id, x, y, z);
            }
        }
        
        // Parse elements (simplified)
        if (inElements) {
            std::istringstream iss(line);
            std::vector<int> values;
            int val;
            while (iss >> val) {
                values.push_back(val);
            }
            
            if (values.size() >= 4) {
                Element element;
                element.id = values[0];
                element.materialId = values[1];
                element.propertyId = values[2];
                
                // Add node IDs
                for (size_t i = 3; i < values.size(); ++i) {
                    element.nodeIds.push_back(values[i]);
                }
                
                // Determine element type based on node count
                switch (element.nodeIds.size()) {
                    case 3: element.type = Element::TRIA3; break;
                    case 4: element.type = Element::QUAD4; break;
                    case 8: element.type = Element::HEXA8; break;
                    default: element.type = Element::UNKNOWN; break;
                }
                
                elements_.push_back(element);
            }
        }
    }
    
    file.close();
    isValid_ = !nodes_.empty();
    
    if (isValid_) {
        std::cout << "Loaded " << nodes_.size() << " nodes and " 
                  << elements_.size() << " elements" << std::endl;
    }
    
    return isValid_;
}

bool RadFileReader::saveFile(const std::string& filename) const {
    if (!isValid_) return false;
    
    std::ofstream file(filename);
    if (!file.is_open()) return false;
    
    file << "#RADIOSS STARTER\n";
    file << "/BEGIN\n";
    
    if (!title_.empty()) {
        file << "/TITLE\n" << title_ << "\n";
    }
    
    // Write nodes
    if (!nodes_.empty()) {
        file << "/NODE\n";
        for (const auto& node : nodes_) {
            file << node.id << " " << node.position.x << " " 
                 << node.position.y << " " << node.position.z << "\n";
        }
    }
    
    // Write elements
    if (!elements_.empty()) {
        file << "/SHELL\n";  // Simplified - assume all are shells
        for (const auto& element : elements_) {
            file << element.id << " " << element.materialId << " " 
                 << element.propertyId;
            for (int nodeId : element.nodeIds) {
                file << " " << nodeId;
            }
            file << "\n";
        }
    }
    
    file << "/END\n";
    file.close();
    return true;
}

void RadFileReader::clear() {
    nodes_.clear();
    elements_.clear();
    title_.clear();
    isValid_ = false;
    lastError_.clear();
}

const Node* RadFileReader::findNode(int id) const {
    auto it = std::find_if(nodes_.begin(), nodes_.end(),
        [id](const Node& node) { return node.id == id; });
    return (it != nodes_.end()) ? &(*it) : nullptr;
}

const Element* RadFileReader::findElement(int id) const {
    auto it = std::find_if(elements_.begin(), elements_.end(),
        [id](const Element& element) { return element.id == id; });
    return (it != elements_.end()) ? &(*it) : nullptr;
}

std::pair<glm::vec3, glm::vec3> RadFileReader::getBoundingBox() const {
    if (nodes_.empty()) {
        return std::make_pair(glm::vec3(0.0f), glm::vec3(0.0f));
    }
    
    glm::vec3 minPos = nodes_[0].position;
    glm::vec3 maxPos = nodes_[0].position;
    
    for (const auto& node : nodes_) {
        minPos = glm::min(minPos, node.position);
        maxPos = glm::max(maxPos, node.position);
    }
    
    return std::make_pair(minPos, maxPos);
}

} // namespace OpenRadiossGUI
