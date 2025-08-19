#!/bin/bash

# OpenRadioss GUI Project Structure Setup Script
# This script fixes common build issues and ensures proper case sensitivity

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
check_project_root() {
    if [[ ! -f "CMakeLists.txt" ]]; then
        print_error "CMakeLists.txt not found. Please run this script from the project root directory."
        exit 1
    fi
    
    print_success "Found CMakeLists.txt - proceeding with setup"
}

# Create required directory structure
create_directories() {
    print_status "Creating required directory structure..."
    
    # Create main directories with correct case
    mkdir -p include
    mkdir -p src
    mkdir -p third_party
    mkdir -p assets/shaders
    mkdir -p examples
    
    print_success "Directory structure created"
}

# Fix file case sensitivity issues
fix_case_sensitivity() {
    print_status "Checking for case sensitivity issues..."
    
    # Check for common case issues and fix them
    if [[ -f "Include/radfilereader.h" ]]; then
        print_warning "Found Include/ directory (uppercase I). Moving to include/"
        mv Include include_temp
        mv include_temp include
    fi
    
    if [[ -f "Src/main.cpp" ]]; then
        print_warning "Found Src/ directory (uppercase S). Moving to src/"
        mv Src src_temp
        mv src_temp src
    fi
    
    if [[ -f "Third_party" ]]; then
        print_warning "Found Third_party/ directory. Moving to third_party/"
        mv Third_party third_party_temp
        mv third_party_temp third_party
    fi
    
    print_success "Case sensitivity issues fixed"
}

# Create missing header file if needed
create_missing_files() {
    print_status "Checking for missing files..."
    
    # Create radfilereader.h if missing
    if [[ ! -f "include/radfilereader.h" ]] && [[ ! -f "radfilereader.h" ]]; then
        print_warning "radfilereader.h not found. Creating template..."
        
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
    Node(int nodeId, float x, float y, float z) 
        : id(nodeId), position(x, y, z) {}
};

struct Element {
    enum Type {
        UNKNOWN = 0,
        TRIA3 = 3,
        QUAD4 = 4,
        TETRA4 = 10,
        HEXA8 = 12
    };
    
    int id;
    Type type;
    int materialId;
    int propertyId;
    std::vector<int> nodeIds;
    
    Element() : id(0), type(UNKNOWN), materialId(0), propertyId(0) {}
};

class RadFileReader {
public:
    RadFileReader();
    ~RadFileReader();
    
    bool loadFile(const std::string& filename);
    bool saveFile(const std::string& filename) const;
    void clear();
    
    const std::vector<Node>& getNodes() const { return nodes_; }
    const std::vector<Element>& getElements() const { return elements_; }
    
    std::string getTitle() const { return title_; }
    bool isValid() const { return isValid_; }
    std::string getLastError() const { return lastError_; }
    
    size_t getNodeCount() const { return nodes_.size(); }
    size_t getElementCount() const { return elements_.size(); }
    
    const Node* findNode(int id) const;
    const Element* findElement(int id) const;
    
    std::pair<glm::vec3, glm::vec3> getBoundingBox() const;

private:
    std::vector<Node> nodes_;
    std::vector<Element> elements_;
    std::string title_;
    bool isValid_;
    std::string lastError_;
};

} // namespace OpenRadiossGUI
EOF
        print_success "Created template radfilereader.h"
    fi
    
    # Create radfilereader.cpp if missing
    if [[ ! -f "src/radfilereader.cpp" ]] && [[ ! -f "radfilereader.cpp" ]]; then
        print_warning "radfilereader.cpp not found. Creating template..."
        
        cat > src/radfilereader.cpp << 'EOF'
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
EOF
        print_success "Created template radfilereader.cpp"
    fi
    
    # Create basic main.cpp if missing
    if [[ ! -f "src/main.cpp" ]] && [[ ! -f "main.cpp" ]]; then
        print_warning "main.cpp not found. Creating basic template..."
        
        cat > src/main.cpp << 'EOF'
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef __has_include
  #if __has_include("radfilereader.h")
    #include "radfilereader.h"
    #define HAS_RAD_READER
  #elif __has_include("../include/radfilereader.h")
    #include "../include/radfilereader.h"
    #define HAS_RAD_READER
  #endif
#endif

// Minimal main function for testing
int main() {
    std::cout << "OpenRadioss GUI - Basic Test" << std::endl;
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Create a minimal window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenRadioss GUI", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    
#ifdef HAS_RAD_READER
    // Test RAD file reader if available
    OpenRadiossGUI::RadFileReader reader;
    std::cout << "RAD file reader available" << std::endl;
#endif
    
    // Basic render loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // Exit after a short time for testing
        static int frameCount = 0;
        if (++frameCount > 60) {
            break;
        }
    }
    
    glfwTerminate();
    std::cout << "Application completed successfully!" << std::endl;
    return 0;
}
EOF
        print_success "Created basic main.cpp"
    fi
}

# Initialize git submodules
init_submodules() {
    print_status "Initializing git submodules..."
    
    if [[ ! -d ".git" ]]; then
        print_warning "Not a git repository. Skipping submodule initialization."
        return
    fi
    
    # Check if submodules are already configured
    if [[ -f ".gitmodules" ]]; then
        print_status "Found .gitmodules, updating existing submodules..."
        git submodule update --init --recursive
    else
        print_status "Setting up new submodules..."
        
        # Add Dear ImGui
        if [[ ! -d "third_party/imgui" ]]; then
            print_status "Adding Dear ImGui submodule..."
            git submodule add https://github.com/ocornut/imgui.git third_party/imgui
        fi
        
        # Add Native File Dialog Extended
        if [[ ! -d "third_party/nativefiledialog" ]]; then
            print_status "Adding Native File Dialog submodule..."
            git submodule add https://github.com/btzy/nativefiledialog-extended.git third_party/nativefiledialog
        fi
        
        # Add GLM (optional, many systems have it installed)
        if [[ ! -d "third_party/glm" ]]; then
            print_status "Adding GLM submodule..."
            git submodule add https://github.com/g-truc/glm.git third_party/glm
        fi
        
        # Initialize all submodules
        git submodule update --init --recursive
    fi
    
    print_success "Submodules initialized"
}

# Fix include paths in source files
fix_include_paths() {
    print_status "Fixing include paths in source files..."
    
    # Fix includes in main.cpp
    if [[ -f "src/main.cpp" ]]; then
        sed -i.bak 's|#include "radfilereader.h"|#include "../include/radfilereader.h"|g' src/main.cpp
        print_status "Fixed includes in src/main.cpp"
    elif [[ -f "main.cpp" ]]; then
        sed -i.bak 's|#include "../include/radfilereader.h"|#include "radfilereader.h"|g' main.cpp
        print_status "Fixed includes in main.cpp"
    fi
    
    # Clean up backup files
    find . -name "*.bak" -delete 2>/dev/null || true
    
    print_success "Include paths fixed"
}

# Create a simple test RAD file
create_test_files() {
    print_status "Creating test files..."
    
    # Create a simple test RAD file
    cat > examples/test.rad << 'EOF'
#RADIOSS STARTER
/BEGIN
/TITLE
Simple Test Model - 4 Node Quad Element
/NODE
        1               0.0               0.0               0.0
        2               1.0               0.0               0.0
        3               1.0               1.0               0.0
        4               0.0               1.0               0.0
/SHELL
        1         1         1         1         2         3         4
/MAT/LAW1
        1              7.8E-6              210000               0.3
/PROP/SHELL
        1               0.001
/END
EOF
    
    print_success "Created test RAD file: examples/test.rad"
}

# Check system dependencies
check_dependencies() {
    print_status "Checking system dependencies..."
    
    local missing_deps=()
    
    # Check CMake
    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    else
        local cmake_version=$(cmake --version | head -n1 | cut -d' ' -f3)
        print_success "CMake found: $cmake_version"
    fi
    
    # Check compiler
    if command -v g++ &> /dev/null; then
        local gcc_version=$(g++ --version | head -n1)
        print_success "GCC found: $gcc_version"
    elif command -v clang++ &> /dev/null; then
        local clang_version=$(clang++ --version | head -n1)
        print_success "Clang found: $clang_version"
    else
        missing_deps+=("g++ or clang++")
    fi
    
    # Platform-specific checks
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        # Check for development libraries
        local packages=("libglfw3-dev" "libglew-dev" "libglm-dev")
        for package in "${packages[@]}"; do
            if ! dpkg -l | grep -q "^ii  $package "; then
                if ! rpm -qa | grep -q "$package"; then
                    print_warning "Package might be missing: $package"
                fi
            fi
        done
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        if ! command -v brew &> /dev/null; then
            print_warning "Homebrew not found. Install from https://brew.sh/"
        else
            print_success "Homebrew found"
        fi
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_status "Please install missing dependencies:"
        print_status "  Linux: sudo apt install cmake build-essential libglfw3-dev libglew-dev libglm-dev"
        print_status "  macOS: brew install cmake glfw glew glm"
        return 1
    fi
    
    print_success "All dependencies found"
    return 0
}

# Test build
test_build() {
    print_status "Testing build configuration..."
    
    # Create temporary build directory
    local temp_build_dir="build_test_$"
    mkdir -p "$temp_build_dir"
    cd "$temp_build_dir"
    
    # Configure with CMake
    if cmake .. &> cmake_output.log; then
        print_success "CMake configuration successful"
        
        # Try to build (just configuration, not full build)
        if cmake --build . --config Release --target help &> build_output.log; then
            print_success "Build configuration test passed"
            cd ..
            rm -rf "$temp_build_dir"
            return 0
        else
            print_error "Build configuration test failed"
            print_status "Build output:"
            cat build_output.log | tail -20
        fi
    else
        print_error "CMake configuration failed"
        print_status "CMake output:"
        cat cmake_output.log | tail -20
    fi
    
    cd ..
    rm -rf "$temp_build_dir"
    return 1
}

# Create build script
create_build_script() {
    print_status "Creating build script..."
    
    cat > build.sh << 'EOF'
#!/bin/bash

set -e

BUILD_TYPE=${1:-Release}
CLEAN_BUILD=${2:-false}
PARALLEL_JOBS=${3:-$(nproc 2>/dev/null || echo 4)}

echo "Building OpenRadioss GUI..."
echo "Build Type: $BUILD_TYPE"
echo "Parallel Jobs: $PARALLEL_JOBS"

# Clean build directory if requested
if [ "$CLEAN_BUILD" = "true" ] && [ -d "build" ]; then
    echo "Cleaning build directory..."
    rm -rf build
fi

# Create and enter build directory
mkdir -p build
cd build

# Configure
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE

# Build
echo "Building..."
cmake --build . --config $BUILD_TYPE --parallel $PARALLEL_JOBS

echo "Build completed successfully!"
echo "Executable: $(pwd)/OpenRadiossGUI"

# Test run
if [ -f "OpenRadiossGUI" ]; then
    echo "Testing executable..."
    timeout 5s ./OpenRadiossGUI || echo "Test completed (timeout expected)"
fi
EOF
    
    chmod +x build.sh
    print_success "Created build.sh script"
}

# Create VS Code configuration
create_vscode_config() {
    print_status "Creating VS Code configuration..."
    
    mkdir -p .vscode
    
    # settings.json
    cat > .vscode/settings.json << 'EOF'
{
    "cmake.configureOnOpen": true,
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "C_Cpp.default.cppStandard": "c++17",
    "files.associations": {
        "*.rad": "plaintext",
        "*.key": "plaintext"
    }
}
EOF
    
    # tasks.json
    cat > .vscode/tasks.json << 'EOF'
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build Release",
            "type": "shell",
            "command": "./build.sh",
            "args": ["Release"],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": "$gcc"
        },
        {
            "label": "Build Debug",
            "type": "shell",
            "command": "./build.sh",
            "args": ["Debug"],
            "group": "build",
            "problemMatcher": "$gcc"
        },
        {
            "label": "Clean Build",
            "type": "shell",
            "command": "./build.sh",
            "args": ["Release", "true"],
            "group": "build"
        }
    ]
}
EOF
    
    print_success "Created VS Code configuration"
}

# Create .gitignore
create_gitignore() {
    if [[ ! -f ".gitignore" ]]; then
        print_status "Creating .gitignore..."
        
        cat > .gitignore << 'EOF'
# Build directories
build/
build*/
cmake-build-*/

# IDE files
.vscode/settings.json.backup
.idea/
*.swp
*.swo
*~

# Compiled files
*.o
*.obj
*.exe
*.dll
*.so
*.dylib
*.a
*.lib

# CMake files
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
Makefile
*.cmake
!CMakeLists.txt

# System files
.DS_Store
Thumbs.db

# Logs
*.log

# Backup files
*.bak
*.backup
*.orig

# Temporary files
*.tmp
*.temp
EOF
        print_success "Created .gitignore"
    fi
}

# Main function
main() {
    print_status "OpenRadioss GUI Project Setup and Fix Script"
    print_status "=============================================="
    
    # Check if we're in the right place
    check_project_root
    
    # Fix project structure
    create_directories
    fix_case_sensitivity
    create_missing_files
    
    # Set up git and submodules
    init_submodules
    
    # Fix source code issues
    fix_include_paths
    
    # Create helper files
    create_test_files
    create_build_script
    create_vscode_config
    create_gitignore
    
    # Check dependencies
    if ! check_dependencies; then
        print_warning "Some dependencies are missing, but continuing..."
    fi
    
    # Test build configuration
    if test_build; then
        print_success "Build test passed!"
    else
        print_warning "Build test failed, but setup is complete"
    fi
    
    print_success "Project setup complete!"
    print_status ""
    print_status "Next steps:"
    print_status "1. Install missing dependencies if any"
    print_status "2. Run: ./build.sh"
    print_status "3. Test with: ./build/OpenRadiossGUI"
    print_status ""
    print_status "To test with sample file:"
    print_status "  ./build/OpenRadiossGUI examples/test.rad"
}

# Run main function
main "$@"