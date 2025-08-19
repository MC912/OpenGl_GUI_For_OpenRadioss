#!/bin/bash

# Clean fix script for OpenRadioss GUI compilation errors
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_status() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }

print_status "OpenRadioss GUI - Clean Compilation Fix"
print_status "======================================"

# Check if we're in the right directory
if [[ ! -f "CMakeLists.txt" ]]; then
    print_error "CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

# Create backups
print_status "Creating backups..."
[[ -f "src/main.cpp" ]] && cp src/main.cpp src/main.cpp.backup
[[ -f "include/radfilereader.h" ]] && cp include/radfilereader.h include/radfilereader.h.backup
[[ -f "radfilereader.h" ]] && cp radfilereader.h radfilereader.h.backup

# Ensure directory structure exists
mkdir -p src include examples

# Fix 1: Add getMaterialCount() to radfilereader.h if missing
print_status "Fixing radfilereader.h..."
if [[ -f "include/radfilereader.h" ]]; then
    if ! grep -q "getMaterialCount" include/radfilereader.h; then
        # Add the missing method after getElementCount line
        sed -i '/getElementCount.*const/a\    size_t getMaterialCount() const { return materials_.size(); }' include/radfilereader.h
        print_success "Added getMaterialCount() to include/radfilereader.h"
    else
        print_status "getMaterialCount() already exists in include/radfilereader.h"
    fi
elif [[ -f "radfilereader.h" ]]; then
    if ! grep -q "getMaterialCount" radfilereader.h; then
        sed -i '/getElementCount.*const/a\    size_t getMaterialCount() const { return materials_.size(); }' radfilereader.h
        print_success "Added getMaterialCount() to radfilereader.h"
    else
        print_status "getMaterialCount() already exists in radfilereader.h"
    fi
else
    print_warning "Creating minimal radfilereader.h..."
    cat > include/radfilereader.h << 'EOF'
#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace OpenRadiossGUI {

struct Node {
    int id;
    glm::vec3 position;
    Node() : id(0), position(0.0f) {}
    Node(int nodeId, float x, float y, float z) : id(nodeId), position(x, y, z) {}
};

struct Element {
    enum Type { UNKNOWN = 0, TRIA3 = 3, QUAD4 = 4, TETRA4 = 10, HEXA8 = 12 };
    int id;
    Type type;
    int materialId, propertyId;
    std::vector<int> nodeIds;
    Element() : id(0), type(UNKNOWN), materialId(0), propertyId(0) {}
};

struct Material {
    int id;
    std::string name, type;
    Material() : id(0) {}
};

class RadFileReader {
public:
    RadFileReader() : isValid_(false) {}
    ~RadFileReader() {}
    
    bool loadFile(const std::string& filename);
    bool saveFile(const std::string& filename) const { return false; }
    void clear() { nodes_.clear(); elements_.clear(); materials_.clear(); isValid_ = false; }
    
    const std::vector<Node>& getNodes() const { return nodes_; }
    const std::vector<Element>& getElements() const { return elements_; }
    const std::vector<Material>& getMaterials() const { return materials_; }
    
    std::string getTitle() const { return title_; }
    bool isValid() const { return isValid_; }
    std::string getLastError() const { return lastError_; }
    
    size_t getNodeCount() const { return nodes_.size(); }
    size_t getElementCount() const { return elements_.size(); }
    size_t getMaterialCount() const { return materials_.size(); }
    
    const Node* findNode(int id) const;
    const Element* findElement(int id) const;
    std::pair<glm::vec3, glm::vec3> getBoundingBox() const;

private:
    std::vector<Node> nodes_;
    std::vector<Element> elements_;
    std::vector<Material> materials_;
    std::string title_, lastError_;
    bool isValid_;
};

}
EOF
    print_success "Created minimal radfilereader.h"
fi

# Fix 2: Update main.cpp to fix ImGui compatibility issues
print_status "Fixing main.cpp for ImGui compatibility..."
cat > src/main.cpp << 'EOF'
#include <iostream>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#if __has_include("../include/radfilereader.h")
#include "../include/radfilereader.h"
#elif __has_include("radfilereader.h")
#include "radfilereader.h"
#endif

struct AppState {
    GLFWwindow* window = nullptr;
    int window_width = 1200, window_height = 800;
    
    // Camera
    glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
    float camera_fov = 45.0f;
    float camera_distance = 5.0f;
    float camera_rotation_x = 0.0f;
    float camera_rotation_y = 0.0f;
    
    // UI state
    bool show_file_dialog = false;
    bool show_about = false;
    bool show_stats = false;
    bool show_nodes = true;
    bool show_elements = true;
    bool show_wireframe = false;
    bool show_axes = true;
    glm::vec3 background_color = glm::vec3(0.2f, 0.3f, 0.3f);
    std::string current_file;
    
    // Model data
    OpenRadiossGUI::RadFileReader rad_reader;
    bool model_loaded = false;
    glm::vec3 model_min = glm::vec3(0.0f);
    glm::vec3 model_max = glm::vec3(0.0f);
    
    // OpenGL objects
    GLuint shader_program = 0;
    GLuint node_vao = 0, node_vbo = 0;
    GLuint element_vao = 0, element_vbo = 0, element_ebo = 0;
    GLuint axis_vao = 0, axis_vbo = 0;
    size_t element_index_count = 0;
    size_t node_count = 0;
};

const char* vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 FragColor;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragColor = aColor;
}
)";

const char* fragment_shader_source = R"(
#version 330 core
in vec3 FragColor;
out vec4 color;
void main() {
    color = vec4(FragColor, 1.0);
}
)";

GLuint createShaderProgram() {
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);
    
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return program;
}

void createAxisGeometry(AppState& app) {
    float axis_data[] = {
        0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
    };
    
    glGenVertexArrays(1, &app.axis_vao);
    glGenBuffers(1, &app.axis_vbo);
    glBindVertexArray(app.axis_vao);
    glBindBuffer(GL_ARRAY_BUFFER, app.axis_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis_data), axis_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void loadRADFile(AppState& app, const std::string& filename) {
    if (app.rad_reader.loadFile(filename)) {
        app.current_file = filename;
        app.model_loaded = true;
        
        if (app.rad_reader.getNodeCount() > 0) {
            auto bbox = app.rad_reader.getBoundingBox();
            app.model_min = bbox.first;
            app.model_max = bbox.second;
            glm::vec3 center = (app.model_min + app.model_max) * 0.5f;
            glm::vec3 size = app.model_max - app.model_min;
            float max_size = glm::max(glm::max(size.x, size.y), size.z);
            app.camera_target = center;
            app.camera_distance = max_size * 2.0f;
        }
        
        // Create geometry buffers (simplified)
        const auto& nodes = app.rad_reader.getNodes();
        if (!nodes.empty()) {
            std::vector<float> node_data;
            for (const auto& node : nodes) {
                node_data.insert(node_data.end(), {node.position.x, node.position.y, node.position.z, 1.0f, 1.0f, 0.0f});
            }
            
            glGenVertexArrays(1, &app.node_vao);
            glGenBuffers(1, &app.node_vbo);
            glBindVertexArray(app.node_vao);
            glBindBuffer(GL_ARRAY_BUFFER, app.node_vbo);
            glBufferData(GL_ARRAY_BUFFER, node_data.size() * sizeof(float), node_data.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glBindVertexArray(0);
            app.node_count = nodes.size();
        }
        
        std::cout << "Loaded: " << filename << " (" << app.rad_reader.getNodeCount() << " nodes)" << std::endl;
    } else {
        std::cerr << "Failed to load: " << filename << std::endl;
    }
}

void updateCamera(AppState& app) {
    float x = app.camera_distance * cos(app.camera_rotation_y) * cos(app.camera_rotation_x);
    float y = app.camera_distance * sin(app.camera_rotation_x);
    float z = app.camera_distance * sin(app.camera_rotation_y) * cos(app.camera_rotation_x);
    app.camera_pos = app.camera_target + glm::vec3(x, y, z);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    AppState* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
            case GLFW_KEY_O: if (mods & GLFW_MOD_CONTROL) app->show_file_dialog = true; break;
            case GLFW_KEY_R: app->camera_rotation_x = app->camera_rotation_y = 0.0f; break;
            case GLFW_KEY_1: app->show_nodes = !app->show_nodes; break;
            case GLFW_KEY_2: app->show_elements = !app->show_elements; break;
            case GLFW_KEY_3: app->show_wireframe = !app->show_wireframe; break;
            case GLFW_KEY_4: app->show_axes = !app->show_axes; break;
        }
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    AppState* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    app->camera_distance *= (1.0f - static_cast<float>(yoffset) * 0.1f);
    app->camera_distance = glm::clamp(app->camera_distance, 0.1f, 1000.0f);
}

int main() {
    AppState app;
    
    if (!glfwInit()) return -1;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    app.window = glfwCreateWindow(app.window_width, app.window_height, "OpenRadioss GUI", nullptr, nullptr);
    if (!app.window) { glfwTerminate(); return -1; }
    
    glfwMakeContextCurrent(app.window);
    glfwSetWindowUserPointer(app.window, &app);
    glfwSetKeyCallback(app.window, keyCallback);
    glfwSetScrollCallback(app.window, scrollCallback);
    
    if (glewInit() != GLEW_OK) return -1;
    
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, app.window_width, app.window_height);
    
    // Setup ImGui (without docking)
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(app.window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    app.shader_program = createShaderProgram();
    createAxisGeometry(app);
    
    std::cout << "OpenRadioss GUI Started!" << std::endl;
    std::cout << "Press Ctrl+O to open a file, or drag mouse to rotate camera" << std::endl;
    
    while (!glfwWindowShouldClose(app.window)) {
        glfwPollEvents();
        updateCamera(app);
        
        glClearColor(app.background_color.r, app.background_color.g, app.background_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open...", "Ctrl+O")) app.show_file_dialog = true;
                if (ImGui::MenuItem("Exit")) glfwSetWindowShouldClose(app.window, GLFW_TRUE);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Show Nodes", "1", &app.show_nodes);
                ImGui::MenuItem("Show Elements", "2", &app.show_elements);
                ImGui::MenuItem("Wireframe", "3", &app.show_wireframe);
                ImGui::MenuItem("Show Axes", "4", &app.show_axes);
                if (ImGui::MenuItem("Statistics")) app.show_stats = true;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) app.show_about = true;
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        
        // Properties panel
        ImGui::Begin("Properties");
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Distance", &app.camera_distance, 0.1f, 100.0f);
            if (ImGui::Button("Reset")) {
                app.camera_rotation_x = app.camera_rotation_y = 0.0f;
                app.camera_distance = 5.0f;
            }
        }
        if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Nodes", &app.show_nodes);
            ImGui::Checkbox("Elements", &app.show_elements);
            ImGui::Checkbox("Wireframe", &app.show_wireframe);
            ImGui::Checkbox("Axes", &app.show_axes);
        }
        if (app.model_loaded && ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Nodes: %zu", app.rad_reader.getNodeCount());
            ImGui::Text("Elements: %zu", app.rad_reader.getElementCount());
            ImGui::Text("Materials: %zu", app.rad_reader.getMaterialCount());
        }
        ImGui::End();
        
        // 3D Viewport
        ImGui::Begin("3D Viewport");
        ImVec2 viewport_size = ImGui::GetContentRegionAvail();
        
        if (viewport_size.x > 0 && viewport_size.y > 0) {
            // Render 3D scene
            float aspect = viewport_size.x / viewport_size.y;
            glm::mat4 projection = glm::perspective(glm::radians(app.camera_fov), aspect, 0.1f, 1000.0f);
            glm::mat4 view = glm::lookAt(app.camera_pos, app.camera_target, app.camera_up);
            glm::mat4 model = glm::mat4(1.0f);
            
            glUseProgram(app.shader_program);
            glUniformMatrix4fv(glGetUniformLocation(app.shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(app.shader_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(app.shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            
            // Render axes
            if (app.show_axes) {
                glLineWidth(3.0f);
                glBindVertexArray(app.axis_vao);
                glDrawArrays(GL_LINES, 0, 6);
                glBindVertexArray(0);
                glLineWidth(1.0f);
            }
            
            // Render nodes
            if (app.show_nodes && app.node_vao != 0) {
                glPointSize(5.0f);
                glBindVertexArray(app.node_vao);
                glDrawArrays(GL_POINTS, 0, app.node_count);
                glBindVertexArray(0);
            }
            
            // Handle mouse interaction
            ImGui::InvisibleButton("3DViewport", viewport_size);
            if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                app.camera_rotation_y += delta.x * 0.01f;
                app.camera_rotation_x += delta.y * 0.01f;
                app.camera_rotation_x = glm::clamp(app.camera_rotation_x, -1.5f, 1.5f);
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
            }
        }
        
        if (!app.model_loaded) {
            ImGui::SetCursorPos(ImVec2(viewport_size.x * 0.5f - 100, viewport_size.y * 0.5f));
            ImGui::Text("No model loaded. Use File -> Open");
        }
        ImGui::End();
        
        // Dialogs
        if (app.show_file_dialog) {
            static char filename[256] = "examples/test.rad";
            ImGui::Begin("Open File", &app.show_file_dialog);
            ImGui::InputText("File", filename, sizeof(filename));
            if (ImGui::Button("Load")) {
                loadRADFile(app, filename);
                app.show_file_dialog = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) app.show_file_dialog = false;
            ImGui::End();
        }
        
        if (app.show_about) {
            ImGui::Begin("About", &app.show_about);
            ImGui::Text("OpenRadioss GUI v1.0");
            ImGui::Text("OpenGL visualization for OpenRadioss");
            ImGui::End();
        }
        
        if (app.show_stats) {
            ImGui::Begin("Statistics", &app.show_stats);
            ImGui::Text("OpenGL: %s", glGetString(GL_VERSION));
            if (app.model_loaded) {
                ImGui::Text("File: %s", app.current_file.c_str());
                ImGui::Text("Nodes: %zu", app.rad_reader.getNodeCount());
                ImGui::Text("Elements: %zu", app.rad_reader.getElementCount());
            }
            ImGui::End();
        }
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(app.window);
    }
    
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
EOF

print_success "Created clean main.cpp without ImGui docking issues"

# Fix 3: Ensure radfilereader.cpp exists and works
print_status "Ensuring radfilereader.cpp exists..."
if [[ ! -f "src/radfilereader.cpp" ]]; then
    cat > src/radfilereader.cpp << 'EOF'
#include "../include/radfilereader.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace OpenRadiossGUI {

bool RadFileReader::loadFile(const std::string& filename) {
    clear();
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        lastError_ = "Cannot open file: " + filename;
        return false;
    }
    
    std::string line;
    bool inNodes = false, inElements = false, inMaterials = false;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == 'C') continue;
        
        if (line.find("/TITLE") != std::string::npos) {
            std::getline(file, title_);
            continue;
        }
        if (line.find("/NODE") != std::string::npos) {
            inNodes = true; inElements = false; inMaterials = false;
            continue;
        }
        if (line.find("/SHELL") != std::string::npos || line.find("/QUAD") != std::string::npos) {
            inNodes = false; inElements = true; inMaterials = false;
            continue;
        }
        if (line.find("/MAT") != std::string::npos) {
            inNodes = false; inElements = false; inMaterials = true;
            continue;
        }
        
        if (inNodes) {
            std::istringstream iss(line);
            int id;
            float x, y, z;
            if (iss >> id >> x >> y >> z) {
                nodes_.emplace_back(id, x, y, z);
            }
        }
        else if (inElements) {
            std::istringstream iss(line);
            std::vector<int> values;
            int val;
            while (iss >> val) values.push_back(val);
            
            if (values.size() >= 4) {
                Element element;
                element.id = values[0];
                element.materialId = values[1];
                element.propertyId = values[2];
                for (size_t i = 3; i < values.size(); ++i) {
                    element.nodeIds.push_back(values[i]);
                }
                element.type = (element.nodeIds.size() == 3) ? Element::TRIA3 : Element::QUAD4;
                elements_.push_back(element);
            }
        }
        else if (inMaterials) {
            std::istringstream iss(line);
            int id;
            if (iss >> id) {
                Material mat;
                mat.id = id;
                mat.type = "LAW1";
                materials_.push_back(mat);
            }
        }
    }
    
    isValid_ = !nodes_.empty();
    return isValid_;
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

}
EOF
    print_success "Created working radfilereader.cpp"
else
    print_status "radfilereader.cpp already exists"
fi

# Fix 4: Create test file
print_status "Creating test RAD file..."
cat > examples/test.rad << 'EOF'
#RADIOSS STARTER
/BEGIN
/TITLE
Simple Test Model
/NODE
        1               0.0               0.0               0.0
        2               1.0               0.0               0.0
        3               1.0               1.0               0.0
        4               0.0               1.0               0.0
        5               0.5               0.5               0.5
/SHELL
        1         1         1         1         2         3         4
/MAT/LAW1
        1              7.8E-6              210000               0.3
/PROP/SHELL
        1               0.001
/END
EOF

print_success "Created examples/test.rad"

print_status ""
print_success "✅ All compilation fixes applied successfully!"
print_status ""
print_status "Changes made:"
print_status "1. ✅ Fixed ImGui compatibility (removed docking features)"
print_status "2. ✅ Added missing getMaterialCount() method"
print_status "3. ✅ Created clean main.cpp without code duplication"
print_status "4. ✅ Ensured radfilereader.cpp implementation exists"
print_status "5. ✅ Created test RAD file"
print_status ""
print_status "Now build the project:"
print_status "  mkdir -p build && cd build"
print_status "  cmake .."
print_status "  make -j4"
print_status ""
print_status "Then run:"
print_status "  ./OpenRadiossGUI"
print_status ""
print_status "Features included:"
print_status "• OpenGL 3D viewport with coordinate axes"
print_status "• ImGui interface with menus and panels"
print_status "• Mouse camera controls (drag to rotate, wheel to zoom)"
print_status "• Keyboard shortcuts (Ctrl+O open, 1-4 toggles, R reset)"
print_status "• RAD file loading and node visualization"
print_status ""
print_success "The GUI should now compile and run without errors!"