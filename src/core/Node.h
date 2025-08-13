#pragma once
#include <glm/glm.hpp>

struct Node {
    int id;
    glm::vec3 position;
    glm::vec3 displacement;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    
    // Boundary conditions
    bool fixedX = false;
    bool fixedY = false;
    bool fixedZ = false;
    
    Node() : id(0), position(0.0f), displacement(0.0f), 
             velocity(0.0f), acceleration(0.0f) {}
    
    Node(int nodeId, const glm::vec3& pos) 
        : id(nodeId), position(pos), displacement(0.0f),
          velocity(0.0f), acceleration(0.0f) {}
};