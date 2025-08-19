#include "RadFileReader.h"
#include <regex>
#include <iomanip>
#include <limits>

namespace OpenRadiossGUI {

RadFileReader::RadFileReader() 
    : isValid_(false) {
    clearError();
}

RadFileReader::~RadFileReader() {
    clear();
}

bool RadFileReader::loadFile(const std::string& filename) {
    clear();
    filename_ = filename;
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        setError("Cannot open file: " + filename);
        return false;
    }
    
    bool success = parseFile(file);
    file.close();
    
    if (success) {
        buildLookupTables();
        isValid_ = validateData();
        if (!isValid_) {
            setError("File validation failed");
        }
    }
    
    return success && isValid_;
}

bool RadFileReader::saveFile(const std::string& filename) const {
    if (!isValid_) {
        return false;
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    bool success = true;
    success &= writeHeader(file);
    success &= writeNodes(file);
    success &= writeElements(file);
    success &= writeMaterials(file);
    success &= writeProperties(file);
    success &= writeLoadCases(file);
    success &= writeBoundaryConditions(file);
    
    file.close();
    return success;
}

void RadFileReader::clear() {
    nodes_.clear();
    elements_.clear();
    materials_.clear();
    properties_.clear();
    loadCases_.clear();
    boundaryConditions_.clear();
    
    title_.clear();
    version_.clear();
    filename_.clear();
    isValid_ = false;
    
    clearLookupTables();
    clearError();
}

bool RadFileReader::parseFile(std::ifstream& file) {
    std::string line;
    ParseState currentState = STATE_HEADER;
    int lineNumber = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        
        // Skip empty lines and comments
        if (isEmpty(line) || isComment(line)) {
            continue;
        }
        
        // Determine section based on keywords
        ParseState newState = determineSection(line);
        if (newState != STATE_UNKNOWN) {
            currentState = newState;
            continue; // Skip the keyword line itself
        }
        
        // Parse content based on current state
        bool parseSuccess = false;
        switch (currentState) {
            case STATE_HEADER:
                parseSuccess = parseHeader(line);
                break;
            case STATE_TITLE:
                parseSuccess = parseTitle(line);
                break;
            case STATE_NODES:
                parseSuccess = parseNode(line);
                break;
            case STATE_ELEMENTS:
                parseSuccess = parseElement(line);
                break;
            case STATE_MATERIALS:
                parseSuccess = parseMaterial(line);
                break;
            case STATE_PROPERTIES:
                parseSuccess = parseProperty(line);
                break;
            case STATE_LOADS:
                parseSuccess = parseLoadCase(line);
                break;
            case STATE_BOUNDARY_CONDITIONS:
                parseSuccess = parseBoundaryCondition(line);
                break;
            default:
                parseSuccess = true; // Skip unknown sections
                break;
        }
        
        if (!parseSuccess) {
            setError("Parse error at line " + std::to_string(lineNumber) + ": " + line);
            return false;
        }
    }
    
    return true;
}

RadFileReader::ParseState RadFileReader::determineSection(const std::string& line) {
    std::string upperLine = line;
    std::transform(upperLine.begin(), upperLine.end(), upperLine.begin(), ::toupper);
    
    if (upperLine.find("/TITLE") != std::string::npos) return STATE_TITLE;
    if (upperLine.find("/NODE") != std::string::npos) return STATE_NODES;
    if (upperLine.find("/CNODE") != std::string::npos) return STATE_NODES;
    if (upperLine.find("/BRICK") != std::string::npos) return STATE_ELEMENTS;
    if (upperLine.find("/HEXA") != std::string::npos) return STATE_ELEMENTS;
    if (upperLine.find("/TETRA4") != std::string::npos) return STATE_ELEMENTS;
    if (upperLine.find("/TETRA10") != std::string::npos) return STATE_ELEMENTS;
    if (upperLine.find("/SHELL") != std::string::npos) return STATE_ELEMENTS;
    if (upperLine.find("/SH3N") != std::string::npos) return STATE_ELEMENTS;
    if (upperLine.find("/QUAD") != std::string::npos) return STATE_ELEMENTS;
    if (upperLine.find("/TRIA") != std::string::npos) return STATE_ELEMENTS;
    if (upperLine.find("/MAT") != std::string::npos) return STATE_MATERIALS;
    if (upperLine.find("/PROP") != std::string::npos) return STATE_PROPERTIES;
    if (upperLine.find("/PART") != std::string::npos) return STATE_PROPERTIES;
    if (upperLine.find("/LOAD") != std::string::npos) return STATE_LOADS;
    if (upperLine.find("/CLOAD") != std::string::npos) return STATE_LOADS;
    if (upperLine.find("/PLOAD") != std::string::npos) return STATE_LOADS;
    if (upperLine.find("/BCS") != std::string::npos) return STATE_BOUNDARY_CONDITIONS;
    if (upperLine.find("/SPC") != std::string::npos) return STATE_BOUNDARY_CONDITIONS;
    if (upperLine.find("/IMPVEL") != std::string::npos) return STATE_BOUNDARY_CONDITIONS;
    
    return STATE_UNKNOWN;
}

bool RadFileReader::parseHeader(const std::string& line) {
    if (line.find("#RADIOSS") != std::string::npos) {
        // Extract version if present
        std::regex versionRegex(R"(version\s*(\d+(?:\.\d+)?))");
        std::smatch match;
        if (std::regex_search(line, match, versionRegex)) {
            version_ = match[1].str();
        }
        return true;
    }
    if (line.find("/BEGIN") != std::string::npos) {
        return true;
    }
    return true; // Accept any header content
}

bool RadFileReader::parseTitle(const std::string& line) {
    if (title_.empty()) {
        title_ = trim(line);
    } else {
        title_ += " " + trim(line);
    }
    return true;
}

bool RadFileReader::parseNode(const std::string& line) {
    auto tokens = tokenizeLine(line);
    if (tokens.size() < 4) {
        return false;
    }
    
    try {
        Node node;
        node.id = std::stoi(tokens[0]);
        node.position.x = std::stof(tokens[1]);
        node.position.y = std::stof(tokens[2]);
        node.position.z = std::stof(tokens[3]);
        
        nodes_.push_back(node);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool RadFileReader::parseElement(const std::string& line) {
    auto tokens = tokenizeLine(line);
    if (tokens.size() < 3) {
        return false;
    }
    
    try {
        Element element;
        element.id = std::stoi(tokens[0]);
        
        // Try to determine element type from context or node count
        size_t nodeCount = tokens.size() - 3; // Subtract ID, material, property
        if (nodeCount >= 2) {
            element.materialId = std::stoi(tokens[1]);
            element.propertyId = std::stoi(tokens[2]);
            
            // Determine element type based on node count
            switch (nodeCount) {
                case 3: element.type = Element::TRIA3; break;
                case 4: 
                    // Could be QUAD4 or TETRA4 - need more context
                    element.type = Element::QUAD4; 
                    break;
                case 5: element.type = Element::PYRAM5; break;
                case 6: element.type = Element::PENTA6; break;
                case 8: element.type = Element::HEXA8; break;
                default: element.type = Element::UNKNOWN; break;
            }
            
            // Extract node IDs
            for (size_t i = 3; i < tokens.size(); ++i) {
                element.nodeIds.push_back(std::stoi(tokens[i]));
            }
        }
        
        elements_.push_back(element);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool RadFileReader::parseMaterial(const std::string& line) {
    auto tokens = tokenizeLine(line);
    if (tokens.size() < 2) {
        return false;
    }
    
    try {
        Material material;
        material.id = std::stoi(tokens[0]);
        
        // Extract material type if present
        if (tokens.size() > 1) {
            material.type = tokens[1];
        }
        
        // Parse material properties (density, young's modulus, etc.)
        for (size_t i = 2; i < tokens.size(); i += 2) {
            if (i + 1 < tokens.size()) {
                std::string propName = tokens[i];
                double propValue = std::stod(tokens[i + 1]);
                material.properties[propName] = propValue;
            }
        }
        
        materials_.push_back(material);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool RadFileReader::parseProperty(const std::string& line) {
    auto tokens = tokenizeLine(line);
    if (tokens.size() < 2) {
        return false;
    }
    
    try {
        Property property;
        property.id = std::stoi(tokens[0]);
        
        if (tokens.size() > 1) {
            property.type = tokens[1];
        }
        
        // Parse property values
        for (size_t i = 2; i < tokens.size(); i += 2) {
            if (i + 1 < tokens.size()) {
                std::string propName = tokens[i];
                double propValue = std::stod(tokens[i + 1]);
                property.values[propName] = propValue;
            }
        }
        
        properties_.push_back(property);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool RadFileReader::parseLoadCase(const std::string& line) {
    auto tokens = tokenizeLine(line);
    if (tokens.size() < 5) {
        return false;
    }
    
    try {
        LoadCase loadCase;
        loadCase.id = std::stoi(tokens[0]);
        loadCase.type = tokens[1];
        loadCase.magnitude = std::stod(tokens[2]);
        loadCase.vector.x = std::stof(tokens[3]);
        loadCase.vector.y = std::stof(tokens[4]);
        if (tokens.size() > 5) {
            loadCase.vector.z = std::stof(tokens[5]);
        }
        
        // Extract affected node IDs
        for (size_t i = 6; i < tokens.size(); ++i) {
            loadCase.nodeIds.push_back(std::stoi(tokens[i]));
        }
        
        loadCases_.push_back(loadCase);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool RadFileReader::parseBoundaryCondition(const std::string& line) {
    auto tokens = tokenizeLine(line);
    if (tokens.size() < 3) {
        return false;
    }
    
    try {
        BoundaryCondition bc;
        bc.id = std::stoi(tokens[0]);
        bc.type = tokens[1];
        
        // Parse DOF constraints
        for (size_t i = 2; i < tokens.size(); ++i) {
            if (tokens[i].find_first_of("123456") != std::string::npos) {
                // DOF specification
                for (char c : tokens[i]) {
                    if (c >= '1' && c <= '6') {
                        bc.dofs.push_back(c - '0');
                    }
                }
            } else {
                // Node ID
                bc.nodeIds.push_back(std::stoi(tokens[i]));
            }
        }
        
        boundaryConditions_.push_back(bc);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::vector<std::string> RadFileReader::tokenizeLine(const std::string& line, char delimiter) const {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        token = trim(token);
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

std::string RadFileReader::trim(const std::string& str) const {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

bool RadFileReader::isComment(const std::string& line) const {
    std::string trimmed = trim(line);
    return trimmed.empty() || trimmed[0] == '#' || trimmed[0] == 'C' || trimmed[0] == 'c';
}

bool RadFileReader::isEmpty(const std::string& line) const {
    return trim(line).empty();
}

const Node* RadFileReader::findNode(int id) const {
    auto it = nodeIdToIndex_.find(id);
    if (it != nodeIdToIndex_.end() && it->second < nodes_.size()) {
        return &nodes_[it->second];
    }
    return nullptr;
}

const Element* RadFileReader::findElement(int id) const {
    auto it = elementIdToIndex_.find(id);
    if (it != elementIdToIndex_.end() && it->second < elements_.size()) {
        return &elements_[it->second];
    }
    return nullptr;
}

const Material* RadFileReader::findMaterial(int id) const {
    auto it = materialIdToIndex_.find(id);
    if (it != materialIdToIndex_.end() && it->second < materials_.size()) {
        return &materials_[it->second];
    }
    return nullptr;
}

const Property* RadFileReader::findProperty(int id) const {
    auto it = propertyIdToIndex_.find(id);
    if (it != propertyIdToIndex_.end() && it->second < properties_.size()) {
        return &properties_[it->second];
    }
    return nullptr;
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

void RadFileReader::addNode(const Node& node) {
    nodes_.push_back(node);
    buildLookupTables();
}

void RadFileReader::addElement(const Element& element) {
    elements_.push_back(element);
    buildLookupTables();
}

void RadFileReader::addMaterial(const Material& material) {
    materials_.push_back(material);
    buildLookupTables();
}

void RadFileReader::addProperty(const Property& property) {
    properties_.push_back(property);
    buildLookupTables();
}

bool RadFileReader::validateData() const {
    return validateNodes() && validateElements() && validateReferences();
}

bool RadFileReader::validateNodes() const {
    std::unordered_set<int> nodeIds;
    for (const auto& node : nodes_) {
        if (nodeIds.count(node.id)) {
            return false; // Duplicate node ID
        }
        nodeIds.insert(node.id);
    }
    return true;
}

bool RadFileReader::validateElements() const {
    std::unordered_set<int> elementIds;
    for (const auto& element : elements_) {
        if (elementIds.count(element.id)) {
            return false; // Duplicate element ID
        }
        elementIds.insert(element.id);
        
        // Validate node count for element type
        int expectedNodes = RadFileUtils::getElementNodeCount(element.type);
        if (expectedNodes > 0 && static_cast<int>(element.nodeIds.size()) != expectedNodes) {
            return false;
        }
    }
    return true;
}

bool RadFileReader::validateReferences() const {
    // Check that all element node references are valid
    for (const auto& element : elements_) {
        for (int nodeId : element.nodeIds) {
            if (!findNode(nodeId)) {
                return false; // Invalid node reference
            }
        }
    }
    return true;
}

void RadFileReader::buildLookupTables() {
    clearLookupTables();
    
    for (size_t i = 0; i < nodes_.size(); ++i) {
        nodeIdToIndex_[nodes_[i].id] = i;
    }
    
    for (size_t i = 0; i < elements_.size(); ++i) {
        elementIdToIndex_[elements_[i].id] = i;
    }
    
    for (size_t i = 0; i < materials_.size(); ++i) {
        materialIdToIndex_[materials_[i].id] = i;
    }
    
    for (size_t i = 0; i < properties_.size(); ++i) {
        propertyIdToIndex_[properties_[i].id] = i;
    }
}

void RadFileReader::clearLookupTables() {
    nodeIdToIndex_.clear();
    elementIdToIndex_.clear();
    materialIdToIndex_.clear();
    propertyIdToIndex_.clear();
}

void RadFileReader::setError(const std::string& error) {
    lastError_ = error;
}

void RadFileReader::clearError() {
    lastError_.clear();
}

bool RadFileReader::writeHeader(std::ofstream& file) const {
    file << "#RADIOSS STARTER\n";
    file << "/BEGIN\n";
    if (!title_.empty()) {
        file << "/TITLE\n" << title_ << "\n";
    }
    return file.good();
}

bool RadFileReader::writeNodes(std::ofstream& file) const {
    if (!nodes_.empty()) {
        file << "/NODE\n";
        for (const auto& node : nodes_) {
            file << std::setw(10) << node.id 
                 << std::setw(20) << std::scientific << std::setprecision(6) << node.position.x
                 << std::setw(20) << std::scientific << std::setprecision(6) << node.position.y
                 << std::setw(20) << std::scientific << std::setprecision(6) << node.position.z
                 << "\n";
        }
    }
    return file.good();
}

bool RadFileReader::writeElements(std::ofstream& file) const {
    if (!elements_.empty()) {
        // Group elements by type and write appropriate headers
        std::unordered_map<Element::Type, std::vector<const Element*>> elementsByType;
        for (const auto& element : elements_) {
            elementsByType[element.type].push_back(&element);
        }
        
        for (const auto& pair : elementsByType) {
            Element::Type type = pair.first;
            const auto& elements = pair.second;
            
            // Write appropriate header for element type
            switch (type) {
                case Element::TRIA3:
                    file << "/SH3N\n";
                    break;
                case Element::QUAD4:
                    file << "/SHELL\n";
                    break;
                case Element::TETRA4:
                    file << "/TETRA4\n";
                    break;
                case Element::HEXA8:
                    file << "/BRICK\n";
                    break;
                case Element::PENTA6:
                    file << "/PENTA6\n";
                    break;
                case Element::PYRAM5:
                    file << "/PYRAM5\n";
                    break;
                default:
                    file << "/SHELL\n"; // Default to shell
                    break;
            }
            
            for (const Element* element : elements) {
                file << std::setw(10) << element->id
                     << std::setw(10) << element->materialId
                     << std::setw(10) << element->propertyId;
                
                for (int nodeId : element->nodeIds) {
                    file << std::setw(10) << nodeId;
                }
                file << "\n";
            }
        }
    }
    return file.good();
}

bool RadFileReader::writeMaterials(std::ofstream& file) const {
    if (!materials_.empty()) {
        for (const auto& material : materials_) {
            file << "/MAT/" << material.type << "\n";
            file << std::setw(10) << material.id;
            
            // Write material properties
            for (const auto& prop : material.properties) {
                file << std::setw(20) << std::scientific << std::setprecision(6) << prop.second;
            }
            file << "\n";
        }
    }
    return file.good();
}

bool RadFileReader::writeProperties(std::ofstream& file) const {
    if (!properties_.empty()) {
        for (const auto& property : properties_) {
            file << "/PROP/" << property.type << "\n";
            file << std::setw(10) << property.id;
            
            // Write property values
            for (const auto& val : property.values) {
                file << std::setw(20) << std::scientific << std::setprecision(6) << val.second;
            }
            file << "\n";
        }
    }
    return file.good();
}

bool RadFileReader::writeLoadCases(std::ofstream& file) const {
    if (!loadCases_.empty()) {
        for (const auto& loadCase : loadCases_) {
            file << "/CLOAD\n";
            file << std::setw(10) << loadCase.id
                 << std::setw(20) << std::scientific << std::setprecision(6) << loadCase.magnitude
                 << std::setw(20) << std::scientific << std::setprecision(6) << loadCase.vector.x
                 << std::setw(20) << std::scientific << std::setprecision(6) << loadCase.vector.y
                 << std::setw(20) << std::scientific << std::setprecision(6) << loadCase.vector.z;
            
            for (int nodeId : loadCase.nodeIds) {
                file << std::setw(10) << nodeId;
            }
            file << "\n";
        }
    }
    return file.good();
}

bool RadFileReader::writeBoundaryConditions(std::ofstream& file) const {
    if (!boundaryConditions_.empty()) {
        for (const auto& bc : boundaryConditions_) {
            file << "/BCS\n";
            file << std::setw(10) << bc.id;
            
            // Write DOF constraints
            for (int dof : bc.dofs) {
                file << dof;
            }
            file << std::setw(10) << "";
            
            for (int nodeId : bc.nodeIds) {
                file << std::setw(10) << nodeId;
            }
            file << "\n";
        }
    }
    return file.good();
}

// RadFileUtils implementation
namespace RadFileUtils {

glm::vec3 convertCoordinates(const glm::vec3& coords, const std::string& fromSystem, const std::string& toSystem) {
    // Simple coordinate system conversion (can be extended)
    if (fromSystem == toSystem) {
        return coords;
    }
    
    // Example: Convert from Y-up to Z-up
    if (fromSystem == "Y_UP" && toSystem == "Z_UP") {
        return glm::vec3(coords.x, -coords.z, coords.y);
    }
    
    // Default: no conversion
    return coords;
}

double convertUnits(double value, const std::string& fromUnit, const std::string& toUnit) {
    // Simple unit conversion (can be extended)
    if (fromUnit == toUnit) {
        return value;
    }
    
    // Length conversions
    if (fromUnit == "mm" && toUnit == "m") {
        return value * 0.001;
    }
    if (fromUnit == "m" && toUnit == "mm") {
        return value * 1000.0;
    }
    if (fromUnit == "in" && toUnit == "mm") {
        return value * 25.4;
    }
    if (fromUnit == "mm" && toUnit == "in") {
        return value / 25.4;
    }
    
    // Default: no conversion
    return value;
}

bool isRadFile(const std::string& filename) {
    std::string extension = filename.substr(filename.find_last_of('.') + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return extension == "rad" || extension == "key" || extension == "k";
}

std::string detectFileFormat(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "UNKNOWN";
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("#RADIOSS") != std::string::npos) {
            return "RADIOSS";
        }
        if (line.find("*KEYWORD") != std::string::npos) {
            return "LS-DYNA";
        }
        if (line.find("/BEGIN") != std::string::npos) {
            return "RADIOSS";
        }
    }
    
    return "UNKNOWN";
}

std::string elementTypeToString(Element::Type type) {
    switch (type) {
        case Element::TRIA3: return "TRIA3";
        case Element::QUAD4: return "QUAD4";
        case Element::TETRA4: return "TETRA4";
        case Element::HEXA8: return "HEXA8";
        case Element::PENTA6: return "PENTA6";
        case Element::PYRAM5: return "PYRAM5";
        default: return "UNKNOWN";
    }
}

Element::Type stringToElementType(const std::string& typeStr) {
    std::string upper = typeStr;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    
    if (upper == "TRIA3" || upper == "TRI3") return Element::TRIA3;
    if (upper == "QUAD4" || upper == "QUAD") return Element::QUAD4;
    if (upper == "TETRA4" || upper == "TET4") return Element::TETRA4;
    if (upper == "HEXA8" || upper == "HEX8") return Element::HEXA8;
    if (upper == "PENTA6" || upper == "PENT6") return Element::PENTA6;
    if (upper == "PYRAM5" || upper == "PYR5") return Element::PYRAM5;
    
    return Element::UNKNOWN;
}

int getElementNodeCount(Element::Type type) {
    switch (type) {
        case Element::TRIA3: return 3;
        case Element::QUAD4: return 4;
        case Element::TETRA4: return 4;
        case Element::PYRAM5: return 5;
        case Element::PENTA6: return 6;
        case Element::HEXA8: return 8;
        default: return 0;
    }
}

double calculateElementQuality(const Element& element, const std::vector<Node>& nodes) {
    // Simple quality metric based on aspect ratio
    if (element.nodeIds.size() < 3) {
        return 0.0;
    }
    
    // Find nodes for this element
    std::vector<glm::vec3> positions;
    for (int nodeId : element.nodeIds) {
        bool found = false;
        for (const auto& node : nodes) {
            if (node.id == nodeId) {
                positions.push_back(node.position);
                found = true;
                break;
            }
        }
        if (!found) {
            return 0.0; // Invalid element
        }
    }
    
    // Calculate basic quality metric
    if (positions.size() >= 3) {
        // For triangular elements, calculate area-based quality
        glm::vec3 v1 = positions[1] - positions[0];
        glm::vec3 v2 = positions[2] - positions[0];
        float area = 0.5f * glm::length(glm::cross(v1, v2));
        
        if (area > 1e-10f) {
            // Calculate perimeter
            float perimeter = glm::length(v1) + glm::length(v2) + glm::length(positions[2] - positions[1]);
            
            // Quality = 4 * sqrt(3) * area / perimeter^2 (for triangles)
            float quality = 4.0f * std::sqrt(3.0f) * area / (perimeter * perimeter);
            return std::min(1.0, static_cast<double>(quality));
        }
    }
    
    return 0.0;
}

bool isElementDegenerate(const Element& element, const std::vector<Node>& nodes) {
    return calculateElementQuality(element, nodes) < 1e-6;
}

} // namespace RadFileUtils

} // namespace OpenRadiossGUI