# OpenRadioss Pre-Processor

A modern, cross-platform pre-processor and visualization tool for OpenRadioss finite element solver.

## Features

- **Model Visualization**: 3D rendering of nodes, elements, and meshes
- **File I/O**: Import/export RAD files with full format support
- **GUI Interface**: Intuitive interface built with Dear ImGui
- **Solver Integration**: Direct integration with OpenRadioss solver
- **Job Management**: Submit and monitor simulation jobs
- **Cross-Platform**: Supports Windows, Linux, and macOS

## Building

### Requirements

- CMake 3.16+
- C++17 compatible compiler
- OpenGL 3.3+
- OpenRadioss installation (for solver functionality)

### Build Instructions

```bash
# Clone repository
git clone https://github.com/yourusername/openradioss-preprocessor.git
cd openradioss-preprocessor

# Initialize submodules
git submodule update --init --recursive

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build . --config Release

# Run
./OpenRadiossPreProcessor