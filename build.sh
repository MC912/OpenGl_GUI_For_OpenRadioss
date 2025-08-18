#!/bin/bash
# build.sh - Build script for OpenRadioss Pre-Processor

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
BUILD_TYPE="Release"
BUILD_DIR="build"
INSTALL_PREFIX="/usr/local"
PARALLEL_JOBS=$(nproc)

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            echo -e "${YELLOW}Cleaning build directory...${NC}"
            rm -rf ${BUILD_DIR}
            shift
            ;;
        --prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --debug          Build in debug mode"
            echo "  --clean          Clean build directory"
            echo "  --prefix PATH    Set installation prefix (default: /usr/local)"
            echo "  --help           Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Check dependencies
echo -e "${GREEN}Checking dependencies...${NC}"

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}CMake is not installed. Please install CMake 3.16 or higher.${NC}"
    exit 1
fi

# Check for compiler
if ! command -v g++ &> /dev/null; then
    echo -e "${RED}g++ is not installed. Please install a C++ compiler.${NC}"
    exit 1
fi

# Check for OpenGL
if ! pkg-config --exists gl; then
    echo -e "${YELLOW}Warning: OpenGL development files not found.${NC}"
    echo "Please install: libgl1-mesa-dev (Ubuntu) or mesa-libGL-devel (Fedora)"
fi

# Initialize git submodules
if [ -d ".git" ]; then
    echo -e "${GREEN}Initializing git submodules...${NC}"
    git submodule update --init --recursive
fi

# Download dependencies if not present
if [ ! -d "dependencies/glfw" ]; then
    echo -e "${GREEN}Downloading GLFW...${NC}"
    git clone https://github.com/glfw/glfw.git dependencies/glfw
fi

if [ ! -d "dependencies/glew" ]; then
    echo -e "${GREEN}Downloading GLEW...${NC}"
    git clone https://github.com/nigels-com/glew.git dependencies/glew
fi

if [ ! -d "dependencies/glm" ]; then
    echo -e "${GREEN}Downloading GLM...${NC}"
    git clone https://github.com/g-truc/glm.git dependencies/glm
fi

if [ ! -d "dependencies/imgui" ]; then
    echo -e "${GREEN}Downloading Dear ImGui...${NC}"
    git clone https://github.com/ocornut/imgui.git dependencies/imgui
fi

if [ ! -d "dependencies/json" ]; then
    echo -e "${GREEN}Downloading JSON library...${NC}"
    git clone https://github.com/nlohmann/json.git dependencies/json
fi

# Create build directory
echo -e "${GREEN}Creating build directory...${NC}"
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

# Configure with CMake
echo -e "${GREEN}Configuring with CMake...${NC}"
cmake .. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} \
    -DBUILD_TESTS=ON \
    -DBUILD_EXAMPLES=ON

if [ $? -ne 0 ]; then
    echo -e "${RED}CMake configuration failed!${NC}"
    exit 1
fi

# Build
echo -e "${GREEN}Building with ${PARALLEL_JOBS} parallel jobs...${NC}"
cmake --build . --config ${BUILD_TYPE} -j ${PARALLEL_JOBS}

if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${GREEN}Executable: ${BUILD_DIR}/OpenRadiossPreProcessor${NC}"

# Optional: Run tests
if [ -f "tests/OpenRadiossPreProcessorTests" ]; then
    echo -e "${GREEN}Running tests...${NC}"
    ctest --output-on-failure
fi

echo -e "${GREEN}Done!${NC}"
