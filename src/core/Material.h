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
