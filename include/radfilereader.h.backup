#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <glm/glm.hpp>

namespace OpenRadiossGUI {

// Forward declarations
struct Node;
struct Element;
struct Material;
struct Property;
struct LoadCase;
struct BoundaryCondition;

// Data structures for OpenRadioss entities
struct Node {
    int id;
    glm::vec3 position;
    std::vector<int> dof_constraints; // Degree of freedom constraints
    
    Node() : id(0), position(0.0f) {}
    Node(int nodeId, float x, float y, float z) 
        : id(nodeId), position(x, y, z) {}
};

struct Element {
    enum Type {
        UNKNOWN = 0,
        TRIA3 = 3,   // 3-node triangle
        QUAD4 = 4,   // 4-node quadrilateral
        TETRA4 = 10, // 4-node tetrahedron
        HEXA8 = 12,  // 8-node hexahedron
        PENTA6 = 13, // 6-node pentahedron
        PYRAM5 = 14  // 5-node pyramid
    };
    
    int id;
    Type type;
    int materialId;
    int propertyId;
    std::vector<int> nodeIds;
    
    Element() : id(0), type(UNKNOWN), materialId(0), propertyId(0) {}
};

struct Material {
    int id;
    std::string name;
    std::string type; // LAW1, LAW2, etc.
    std::unordered_map<std::string, double> properties;
    
    Material() : id(0) {}
};

struct Property {
    int id;
    std::string name;
    std::string type; // SHELL, SOLID, etc.
    std::unordered_map<std::string, double> values;
    
    Property() : id(0) {}
};

struct LoadCase {
    int id;
    std::string name;
    std::string type; // FORCE, MOMENT, PRESSURE, etc.
    std::vector<int> nodeIds;
    glm::vec3 vector;
    double magnitude;
    
    LoadCase() : id(0), vector(0.0f), magnitude(0.0) {}
};

struct BoundaryCondition {
    int id;
    std::string type; // SPC, MPC, etc.
    std::vector<int> nodeIds;
    std::vector<int> dofs; // Constrained degrees of freedom
    
    BoundaryCondition() : id(0) {}
};

// Main RAD file reader class
class RadFileReader {
public:
    RadFileReader();
    ~RadFileReader();
    
    // Main interface methods
    bool loadFile(const std::string& filename);
    bool saveFile(const std::string& filename) const;
    void clear();
    
    // Getters for data access
    const std::vector<Node>& getNodes() const { return nodes_; }
    const std::vector<Element>& getElements() const { return elements_; }
    const std::vector<Material>& getMaterials() const { return materials_; }
    const std::vector<Property>& getProperties() const { return properties_; }
    const std::vector<LoadCase>& getLoadCases() const { return loadCases_; }
    const std::vector<BoundaryCondition>& getBoundaryConditions() const { return boundaryConditions_; }
    
    // Utility methods
    std::string getTitle() const { return title_; }
    std::string getVersion() const { return version_; }
    bool isValid() const { return isValid_; }
    std::string getLastError() const { return lastError_; }
    
    // Statistics - FIXED: Added getMaterialCount() method
    size_t getNodeCount() const { return nodes_.size(); }
    size_t getElementCount() const { return elements_.size(); }
    size_t getMaterialCount() const { return materials_.size(); }
    size_t getPropertyCount() const { return properties_.size(); }
    size_t getLoadCaseCount() const { return loadCases_.size(); }
    size_t getBoundaryConditionCount() const { return boundaryConditions_.size(); }
    
    // Search methods
    const Node* findNode(int id) const;
    const Element* findElement(int id) const;
    const Material* findMaterial(int id) const;
    const Property* findProperty(int id) const;
    
    // Bounding box calculation
    std::pair<glm::vec3, glm::vec3> getBoundingBox() const;
    
    // Add/modify entities (for future editing capabilities)
    void addNode(const Node& node);
    void addElement(const Element& element);
    void addMaterial(const Material& material);
    void addProperty(const Property& property);

private:
    // Internal data storage
    std::vector<Node> nodes_;
    std::vector<Element> elements_;
    std::vector<Material> materials_;
    std::vector<Property> properties_;
    std::vector<LoadCase> loadCases_;
    std::vector<BoundaryCondition> boundaryConditions_;
    
    // File metadata
    std::string title_;
    std::string version_;
    std::string filename_;
    bool isValid_;
    std::string lastError_;
    
    // Parsing state
    enum ParseState {
        STATE_HEADER,
        STATE_TITLE,
        STATE_NODES,
        STATE_ELEMENTS,
        STATE_MATERIALS,
        STATE_PROPERTIES,
        STATE_LOADS,
        STATE_BOUNDARY_CONDITIONS,
        STATE_UNKNOWN
    };
    
    // Internal parsing methods
    bool parseFile(std::ifstream& file);
    ParseState determineSection(const std::string& line);
    bool parseHeader(const std::string& line);
    bool parseTitle(const std::string& line);
    bool parseNode(const std::string& line);
    bool parseElement(const std::string& line);
    bool parseMaterial(const std::string& line);
    bool parseProperty(const std::string& line);
    bool parseLoadCase(const std::string& line);
    bool parseBoundaryCondition(const std::string& line);
    
    // Utility parsing methods
    std::vector<std::string> tokenizeLine(const std::string& line, char delimiter = ' ') const;
    std::string trim(const std::string& str) const;
    bool isComment(const std::string& line) const;
    bool isEmpty(const std::string& line) const;
    Element::Type parseElementType(const std::string& typeStr) const;
    
    // Validation methods
    bool validateData() const;
    bool validateNodes() const;
    bool validateElements() const;
    bool validateReferences() const;
    
    // File writing methods
    bool writeHeader(std::ofstream& file) const;
    bool writeNodes(std::ofstream& file) const;
    bool writeElements(std::ofstream& file) const;
    bool writeMaterials(std::ofstream& file) const;
    bool writeProperties(std::ofstream& file) const;
    bool writeLoadCases(std::ofstream& file) const;
    bool writeBoundaryConditions(std::ofstream& file) const;
    
    // Error handling
    void setError(const std::string& error);
    void clearError();
    
    // Internal maps for fast lookup
    std::unordered_map<int, size_t> nodeIdToIndex_;
    std::unordered_map<int, size_t> elementIdToIndex_;
    std::unordered_map<int, size_t> materialIdToIndex_;
    std::unordered_map<int, size_t> propertyIdToIndex_;
    
    // Build lookup tables
    void buildLookupTables();
    void clearLookupTables();
};

// Utility functions
namespace RadFileUtils {
    // Convert between different coordinate systems if needed
    glm::vec3 convertCoordinates(const glm::vec3& coords, const std::string& fromSystem, const std::string& toSystem);
    
    // Unit conversion utilities
    double convertUnits(double value, const std::string& fromUnit, const std::string& toUnit);
    
    // File format detection
    bool isRadFile(const std::string& filename);
    std::string detectFileFormat(const std::string& filename);
    
    // Element type utilities
    std::string elementTypeToString(Element::Type type);
    Element::Type stringToElementType(const std::string& typeStr);
    int getElementNodeCount(Element::Type type);
    
    // Mesh quality utilities
    double calculateElementQuality(const Element& element, const std::vector<Node>& nodes);
    bool isElementDegenerate(const Element& element, const std::vector<Node>& nodes);
}

} // namespace OpenRadiossGUI