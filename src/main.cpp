#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <nfd.h>

#include "RadFileReader.h"

// Application state
struct AppState {
    GLFWwindow* window = nullptr;
    bool show_file_dialog = false;
    bool show_about = false;
    bool show_stats = false;
    std::string current_file;
    OpenRadiossGUI::RadFileReader rad_reader;
    
    // Camera state
    glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
    float camera_fov = 45.0f;
    
    // Mouse interaction
    bool mouse_dragging = false;
    double last_mouse_x = 0.0;
    double last_mouse_y = 0.0;
    
    // Rendering options
    bool show_nodes = true;
    bool show_elements = true;
    bool show_wireframe = false;
    bool show_bounding_box = false;
    glm::vec3 background_color = glm::vec3(0.2f, 0.3f, 0.3f);
    
    // OpenGL objects
    GLuint node_vao = 0, node_vbo = 0;
    GLuint element_vao = 0, element_vbo = 0, element_ebo = 0;
    GLuint shader_program = 0;
};

// Forward declarations
bool initializeOpenGL(AppState& app);
void setupImGui(AppState& app);
void renderFrame(AppState& app);
void renderMenuBar(AppState& app);
void renderMainView(AppState& app);
void renderSidebar(AppState& app);
void updateCamera(AppState& app);
void loadRADFile(AppState& app, const std::string& filename);
void createGeometryBuffers(AppState& app);
void renderGeometry(AppState& app);
GLuint createShaderProgram();
void cleanup(AppState& app);

// GLFW callbacks
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void windowSizeCallback(GLFWwindow* window, int width, int height);

// Shader sources
const char* vertexShaderSource = R"(
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

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 FragColor;
out vec4 color;

void main() {
    color = vec4(FragColor, 1.0);
}
)";

int main() {
    AppState app;
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    app.window = glfwCreateWindow(1200, 800, "OpenRadioss GUI - Pre-processor & Visualization", nullptr, nullptr);
    if (!app.window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(app.window);
    glfwSetWindowUserPointer(app.window, &app);
    
    // Set callbacks
    glfwSetKeyCallback(app.window, keyCallback);
    glfwSetMouseButtonCallback(app.window, mouseButtonCallback);
    glfwSetCursorPosCallback(app.window, cursorPosCallback);
    glfwSetScrollCallback(app.window, scrollCallback);
    glfwSetWindowSizeCallback(app.window, windowSizeCallback);
    
    // Initialize OpenGL
    if (!initializeOpenGL(app)) {
        glfwTerminate();
        return -1;
    }
    
    // Initialize ImGui
    setupImGui(app);
    
    // Create shader program
    app.shader_program = createShaderProgram();
    if (app.shader_program == 0) {
        std::cerr << "Failed to create shader program" << std::endl;
        cleanup(app);
        return -1;
    }
    
    // Enable depth testing and multisampling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Main render loop
    while (!glfwWindowShouldClose(app.window)) {
        glfwPollEvents();
        
        updateCamera(app);
        renderFrame(app);
        
        glfwSwapBuffers(app.window);
    }
    
    cleanup(app);
    return 0;
}

bool initializeOpenGL(AppState& app) {
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }
    
    // Check OpenGL version
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    
    return true;
}

void setupImGui(AppState& app) {
    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    // Setup ImGui style
    ImGui::StyleColorsDark();
    
    // Setup platform/renderer backends
    ImGui_ImplGlfw_InitForOpenGL(app.window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void renderFrame(AppState& app) {
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // Enable docking
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    
    // Render UI components
    renderMenuBar(app);
    renderMainView(app);
    renderSidebar(app);
    
    // File dialogs
    if (app.show_file_dialog) {
        nfdchar_t* outPath = nullptr;
        nfdfilteritem_t filterItem[2] = {{"RAD Files", "rad"}, {"All Files", "*"}};
        
        nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 2, nullptr);
        if (result == NFD_OKAY) {
            loadRADFile(app, std::string(outPath));
            free(outPath);
        }
        app.show_file_dialog = false;
    }
    
    // About dialog
    if (app.show_about) {
        ImGui::Begin("About OpenRadioss GUI", &app.show_about);
        ImGui::Text("OpenRadioss GUI");
        ImGui::Text("Version 1.0.0");
        ImGui::Separator();
        ImGui::Text("A modern OpenGL-based pre-processor and");
        ImGui::Text("visualization tool for OpenRadioss finite");
        ImGui::Text("element solver.");
        ImGui::Separator();
        ImGui::Text("Built with:");
        ImGui::BulletText("OpenGL 3.3+");
        ImGui::BulletText("Dear ImGui");
        ImGui::BulletText("GLFW");
        ImGui::BulletText("GLM");
        ImGui::End();
    }
    
    // Statistics dialog
    if (app.show_stats && app.rad_reader.isValid()) {
        ImGui::Begin("Model Statistics", &app.show_stats);
        ImGui::Text("File: %s", app.current_file.c_str());
        ImGui::Separator();
        ImGui::Text("Nodes: %zu", app.rad_reader.getNodeCount());
        ImGui::Text("Elements: %zu", app.rad_reader.getElementCount());
        ImGui::Text("Materials: %zu", app.rad_reader.getMaterialCount());
        
        if (app.rad_reader.getNodeCount() > 0) {
            auto bbox = app.rad_reader.getBoundingBox();
            ImGui::Separator();
            ImGui::Text("Bounding Box:");
            ImGui::Text("Min: %.3f, %.3f, %.3f", bbox.first.x, bbox.first.y, bbox.first.z);
            ImGui::Text("Max: %.3f, %.3f, %.3f", bbox.second.x, bbox.second.y, bbox.second.z);
            
            glm::vec3 size = bbox.second - bbox.first;
            ImGui::Text("Size: %.3f, %.3f, %.3f", size.x, size.y, size.z);
        }
        ImGui::End();
    }
    
    // Clear the framebuffer
    glClearColor(app.background_color.r, app.background_color.g, app.background_color.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Render 3D geometry
    renderGeometry(app);
    
    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // Handle multi-viewport
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void renderMenuBar(AppState& app) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                app.show_file_dialog = true;
            }
            if (ImGui::MenuItem("Save", "Ctrl+S", false, app.rad_reader.isValid())) {
                if (!app.current_file.empty()) {
                    app.rad_reader.saveFile(app.current_file);
                }
            }
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", false, app.rad_reader.isValid())) {
                // TODO: Implement save as dialog
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                glfwSetWindowShouldClose(app.window, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Show Nodes", nullptr, &app.show_nodes);
            ImGui::MenuItem("Show Elements", nullptr, &app.show_elements);
            ImGui::MenuItem("Wireframe", nullptr, &app.show_wireframe);
            ImGui::MenuItem("Bounding Box", nullptr, &app.show_bounding_box);
            ImGui::Separator();
            if (ImGui::MenuItem("Statistics")) {
                app.show_stats = true;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Run OpenRadioss Starter", nullptr, false, app.rad_reader.isValid())) {
                // TODO: Implement solver execution
            }
            if (ImGui::MenuItem("Run OpenRadioss Engine", nullptr, false, app.rad_reader.isValid())) {
                // TODO: Implement solver execution
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                app.show_about = true;
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void renderMainView(AppState& app) {
    ImGui::Begin("3D Viewport");
    
    // Get the content region for the 3D view
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    
    if (viewportSize.x > 0 && viewportSize.y > 0) {
        // Update projection matrix based on viewport size
        float aspect = viewportSize.x / viewportSize.y;
        glm::mat4 projection = glm::perspective(glm::radians(app.camera_fov), aspect, 0.1f, 1000.0f);
        
        // This would be where we render to a framebuffer texture for ImGui integration
        // For now, we'll just reserve the space
        ImGui::InvisibleButton("3DViewport", viewportSize);
        
        // Handle viewport interactions
        if (ImGui::IsItemHovered()) {
            ImGuiIO& io = ImGui::GetIO();
            if (io.MouseWheel != 0.0f) {
                app.camera_fov = glm::clamp(app.camera_fov - io.MouseWheel * 2.0f, 1.0f, 120.0f);
            }
        }
    }
    
    ImGui::End();
}

void renderSidebar(AppState& app) {
    ImGui::Begin("Properties");
    
    if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Show Nodes", &app.show_nodes);
        ImGui::Checkbox("Show Elements", &app.show_elements);
        ImGui::Checkbox("Wireframe Mode", &app.show_wireframe);
        ImGui::Checkbox("Bounding Box", &app.show_bounding_box);
        ImGui::ColorEdit3("Background", glm::value_ptr(app.background_color));
    }
    
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("FOV", &app.camera_fov, 10.0f, 120.0f);
        ImGui::InputFloat3("Position", glm::value_ptr(app.camera_pos));
        ImGui::InputFloat3("Target", glm::value_ptr(app.camera_target));
        
        if (ImGui::Button("Reset Camera")) {
            if (app.rad_reader.isValid() && app.rad_reader.getNodeCount() > 0) {
                auto bbox = app.rad_reader.getBoundingBox();
                glm::vec3 center = (bbox.first + bbox.second) * 0.5f;
                glm::vec3 size = bbox.second - bbox.first;
                float maxSize = glm::max(glm::max(size.x, size.y), size.z);
                
                app.camera_target = center;
                app.camera_pos = center + glm::vec3(0, 0, maxSize * 2.0f);
            } else {
                app.camera_pos = glm::vec3(0.0f, 0.0f, 5.0f);
                app.camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
            }
        }
    }
    
    if (app.rad_reader.isValid() && ImGui::CollapsingHeader("Model Info", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Title: %s", app.rad_reader.getTitle().c_str());
        ImGui::Text("Version: %s", app.rad_reader.getVersion().c_str());
        ImGui::Separator();
        ImGui::Text("Nodes: %zu", app.rad_reader.getNodeCount());
        ImGui::Text("Elements: %zu", app.rad_reader.getElementCount());
        ImGui::Text("Materials: %zu", app.rad_reader.getMaterialCount());
    }
    
    ImGui::End();
}

void updateCamera(AppState& app) {
    // Camera update logic would go here
    // This is a placeholder for camera manipulation
}

void loadRADFile(AppState& app, const std::string& filename) {
    if (app.rad_reader.loadFile(filename)) {
        app.current_file = filename;
        createGeometryBuffers(app);
        
        // Auto-fit camera to model
        if (app.rad_reader.getNodeCount() > 0) {
            auto bbox = app.rad_reader.getBoundingBox();
            glm::vec3 center = (bbox.first + bbox.second) * 0.5f;
            glm::vec3 size = bbox.second - bbox.first;
            float maxSize = glm::max(glm::max(size.x, size.y), size.z);
            
            app.camera_target = center;
            app.camera_pos = center + glm::vec3(0, 0, maxSize * 2.0f);
        }
        
        std::cout << "Successfully loaded: " << filename << std::endl;
        std::cout << "Nodes: " << app.rad_reader.getNodeCount() << std::endl;
        std::cout << "Elements: " << app.rad_reader.getElementCount() << std::endl;
    } else {
        std::cerr << "Failed to load file: " << filename << std::endl;
        std::cerr << "Error: " << app.rad_reader.getLastError() << std::endl;
    }
}

void createGeometryBuffers(AppState& app) {
    if (!app.rad_reader.isValid()) return;
    
    const auto& nodes = app.rad_reader.getNodes();
    const auto& elements = app.rad_reader.getElements();
    
    // Create node geometry
    if (!nodes.empty()) {
        std::vector<float> nodeData;
        for (const auto& node : nodes) {
            // Position
            nodeData.push_back(node.position.x);
            nodeData.push_back(node.position.y);
            nodeData.push_back(node.position.z);
            // Color (white for nodes)
            nodeData.push_back(1.0f);
            nodeData.push_back(1.0f);
            nodeData.push_back(1.0f);
        }
        
        // Clean up existing buffers
        if (app.node_vao != 0) {
            glDeleteVertexArrays(1, &app.node_vao);
            glDeleteBuffers(1, &app.node_vbo);
        }
        
        // Create node VAO and VBO
        glGenVertexArrays(1, &app.node_vao);
        glGenBuffers(1, &app.node_vbo);
        
        glBindVertexArray(app.node_vao);
        glBindBuffer(GL_ARRAY_BUFFER, app.node_vbo);
        glBufferData(GL_ARRAY_BUFFER, nodeData.size() * sizeof(float), nodeData.data(), GL_STATIC_DRAW);
        
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
    }
    
    // Create element geometry
    if (!elements.empty()) {
        std::vector<float> elementData;
        std::vector<unsigned int> indices;
        
        for (const auto& element : elements) {
            if (element.nodeIds.size() < 3) continue; // Skip invalid elements
            
            // Find nodes for this element and add to vertex data
            std::vector<size_t> elementIndices;
            for (int nodeId : element.nodeIds) {
                const auto* node = app.rad_reader.findNode(nodeId);
                if (node) {
                    elementIndices.push_back(elementData.size() / 6);
                    
                    // Position
                    elementData.push_back(node->position.x);
                    elementData.push_back(node->position.y);
                    elementData.push_back(node->position.z);
                    
                    // Color based on element type
                    switch (element.type) {
                        case OpenRadiossGUI::Element::TRIA3:
                            elementData.push_back(0.0f); elementData.push_back(1.0f); elementData.push_back(0.0f);
                            break;
                        case OpenRadiossGUI::Element::QUAD4:
                            elementData.push_back(0.0f); elementData.push_back(0.0f); elementData.push_back(1.0f);
                            break;
                        case OpenRadiossGUI::Element::TETRA4:
                            elementData.push_back(1.0f); elementData.push_back(1.0f); elementData.push_back(0.0f);
                            break;
                        case OpenRadiossGUI::Element::HEXA8:
                            elementData.push_back(1.0f); elementData.push_back(0.0f); elementData.push_back(1.0f);
                            break;
                        default:
                            elementData.push_back(0.7f); elementData.push_back(0.7f); elementData.push_back(0.7f);
                            break;
                    }
                }
            }
            
            // Create triangular faces for rendering
            if (elementIndices.size() >= 3) {
                if (element.type == OpenRadiossGUI::Element::TRIA3) {
                    // Triangle
                    indices.push_back(elementIndices[0]);
                    indices.push_back(elementIndices[1]);
                    indices.push_back(elementIndices[2]);
                } else if (element.type == OpenRadiossGUI::Element::QUAD4) {
                    // Quad -> 2 triangles
                    indices.push_back(elementIndices[0]);
                    indices.push_back(elementIndices[1]);
                    indices.push_back(elementIndices[2]);
                    
                    indices.push_back(elementIndices[0]);
                    indices.push_back(elementIndices[2]);
                    indices.push_back(elementIndices[3]);
                } else if (element.type == OpenRadiossGUI::Element::TETRA4) {
                    // Tetrahedron -> 4 triangular faces
                    int faces[4][3] = {{0,1,2}, {0,1,3}, {0,2,3}, {1,2,3}};
                    for (int i = 0; i < 4; i++) {
                        indices.push_back(elementIndices[faces[i][0]]);
                        indices.push_back(elementIndices[faces[i][1]]);
                        indices.push_back(elementIndices[faces[i][2]]);
                    }
                } else if (element.type == OpenRadiossGUI::Element::HEXA8) {
                    // Hexahedron -> 12 triangular faces (6 quad faces)
                    int faces[6][4] = {
                        {0,1,2,3}, {4,7,6,5}, {0,4,5,1},
                        {2,6,7,3}, {0,3,7,4}, {1,5,6,2}
                    };
                    for (int i = 0; i < 6; i++) {
                        // First triangle
                        indices.push_back(elementIndices[faces[i][0]]);
                        indices.push_back(elementIndices[faces[i][1]]);
                        indices.push_back(elementIndices[faces[i][2]]);
                        // Second triangle
                        indices.push_back(elementIndices[faces[i][0]]);
                        indices.push_back(elementIndices[faces[i][2]]);
                        indices.push_back(elementIndices[faces[i][3]]);
                    }
                }
            }
        }
        
        // Clean up existing buffers
        if (app.element_vao != 0) {
            glDeleteVertexArrays(1, &app.element_vao);
            glDeleteBuffers(1, &app.element_vbo);
            glDeleteBuffers(1, &app.element_ebo);
        }
        
        // Create element VAO, VBO, and EBO
        glGenVertexArrays(1, &app.element_vao);
        glGenBuffers(1, &app.element_vbo);
        glGenBuffers(1, &app.element_ebo);
        
        glBindVertexArray(app.element_vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, app.element_vbo);
        glBufferData(GL_ARRAY_BUFFER, elementData.size() * sizeof(float), elementData.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app.element_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
    }
}

void renderGeometry(AppState& app) {
    if (!app.rad_reader.isValid() || app.shader_program == 0) return;
    
    // Use shader program
    glUseProgram(app.shader_program);
    
    // Set up matrices
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(app.camera_pos, app.camera_target, app.camera_up);
    
    int width, height;
    glfwGetFramebufferSize(app.window, &width, &height);
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    glm::mat4 projection = glm::perspective(glm::radians(app.camera_fov), aspect, 0.1f, 1000.0f);
    
    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(app.shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(app.shader_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(app.shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    // Set polygon mode
    if (app.show_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    // Render elements
    if (app.show_elements && app.element_vao != 0) {
        glBindVertexArray(app.element_vao);
        
        GLint indexCount;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app.element_ebo);
        glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &indexCount);
        indexCount /= sizeof(unsigned int);
        
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    // Render nodes as points
    if (app.show_nodes && app.node_vao != 0) {
        glPointSize(5.0f);
        glBindVertexArray(app.node_vao);
        glDrawArrays(GL_POINTS, 0, app.rad_reader.getNodeCount());
        glBindVertexArray(0);
    }
    
    // Reset polygon mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

GLuint createShaderProgram() {
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    // Check vertex shader compilation
    GLint success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
        return 0;
    }
    
    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
        return 0;
    }
    
    // Create shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // Check program linking
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        return 0;
    }
    
    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

void cleanup(AppState& app) {
    // Clean up OpenGL objects
    if (app.node_vao != 0) {
        glDeleteVertexArrays(1, &app.node_vao);
        glDeleteBuffers(1, &app.node_vbo);
    }
    if (app.element_vao != 0) {
        glDeleteVertexArrays(1, &app.element_vao);
        glDeleteBuffers(1, &app.element_vbo);
        glDeleteBuffers(1, &app.element_ebo);
    }
    if (app.shader_program != 0) {
        glDeleteProgram(app.shader_program);
    }
    
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    // Cleanup GLFW
    glfwTerminate();
}

// GLFW Callbacks
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    AppState* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_O:
                if (mods & GLFW_MOD_CONTROL) {
                    app->show_file_dialog = true;
                }
                break;
            case GLFW_KEY_S:
                if ((mods & GLFW_MOD_CONTROL) && app->rad_reader.isValid()) {
                    if (!app->current_file.empty()) {
                        app->rad_reader.saveFile(app->current_file);
                    }
                }
                break;
        }
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    AppState* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            app->mouse_dragging = true;
            glfwGetCursorPos(window, &app->last_mouse_x, &app->last_mouse_y);
        } else if (action == GLFW_RELEASE) {
            app->mouse_dragging = false;
        }
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    AppState* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    
    if (app->mouse_dragging) {
        double deltaX = xpos - app->last_mouse_x;
        double deltaY = ypos - app->last_mouse_y;
        
        // Simple orbit camera
        const float sensitivity = 0.01f;
        
        // Calculate camera orbit
        glm::vec3 direction = app->camera_pos - app->camera_target;
        float radius = glm::length(direction);
        
        // Horizontal rotation (around Y axis)
        float angleY = static_cast<float>(deltaX) * sensitivity;
        glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), angleY, glm::vec3(0, 1, 0));
        
        // Vertical rotation (around right vector)
        glm::vec3 right = glm::normalize(glm::cross(direction, app->camera_up));
        float angleX = static_cast<float>(deltaY) * sensitivity;
        glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), angleX, right);
        
        // Apply rotations
        glm::vec4 newDirection = rotY * rotX * glm::vec4(direction, 1.0f);
        app->camera_pos = app->camera_target + glm::vec3(newDirection);
        
        app->last_mouse_x = xpos;
        app->last_mouse_y = ypos;
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    AppState* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    
    // Zoom in/out
    glm::vec3 direction = app->camera_pos - app->camera_target;
    float distance = glm::length(direction);
    
    distance *= (1.0f - static_cast<float>(yoffset) * 0.1f);
    distance = glm::clamp(distance, 0.1f, 1000.0f);
    
    app->camera_pos = app->camera_target + glm::normalize(direction) * distance;
}

void windowSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}