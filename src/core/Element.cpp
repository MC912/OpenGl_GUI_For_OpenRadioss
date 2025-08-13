#include "core/Element.h"

ElementType Element::StringToType(const std::string& typeStr) {
    if (typeStr == "SH3N") return ElementType::SHELL3;
    if (typeStr == "SHELL") return ElementType::SHELL4;
    if (typeStr == "TETRA") return ElementType::TETRA4;
    if (typeStr == "BRICK") return ElementType::HEXA8;
    if (typeStr == "BEAM") return ElementType::BEAM2;
    if (typeStr == "SPRING") return ElementType::SPRING1;
    return ElementType::UNKNOWN;
}

std::string Element::TypeToString(ElementType type) {
    switch (type) {
        case ElementType::SHELL3: return "SH3N";
        case ElementType::SHELL4: return "SHELL";
        case ElementType::TETRA4: return "TETRA";
        case ElementType::HEXA8: return "BRICK";
        case ElementType::BEAM2: return "BEAM";
        case ElementType::SPRING1: return "SPRING";
        default: return "UNKNOWN";
    }
}

int Element::GetNodeCount(ElementType type) {
    switch (type) {
        case ElementType::SHELL3: return 3;
        case ElementType::SHELL4: return 4;
        case ElementType::TETRA4: return 4;
        case ElementType::HEXA8: return 8;
        case ElementType::BEAM2: return 2;
        case ElementType::SPRING1: return 1;
        default: return 0;
    }
}
