#pragma once
#include <vector>
#include <string>

enum class ElementType {
    UNKNOWN = 0,
    SHELL3,     // Triangle shell
    SHELL4,     // Quad shell
    TETRA4,     // Tetrahedron
    HEXA8,      // Hexahedron
    BEAM2,      // Beam
    SPRING1     // Spring
};

struct Element {
    int id;
    ElementType type;
    std::vector<int> nodeIds;
    int materialId;
    int propertyId;
    float thickness;  // For shells
    
    Element() : id(0), type(ElementType::UNKNOWN), 
                materialId(0), propertyId(0), thickness(0.0f) {}
    
    static ElementType StringToType(const std::string& typeStr);
    static std::string TypeToString(ElementType type);
    static int GetNodeCount(ElementType type);
};