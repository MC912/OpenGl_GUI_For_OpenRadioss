# OpenRadioss GUI - OpenGL Pre-processor & Visualization Tool

A modern, cross-platform OpenGL-based graphical user interface for the OpenRadioss finite element solver. This tool provides an intuitive interface for pre-processing, visualization, and job management.

## Features

- **3D Visualization**: Real-time OpenGL rendering of finite element models
- **File I/O**: Complete support for OpenRadioss RAD file format
- **Modern GUI**: Built with Dear ImGui for a responsive, immediate-mode interface
- **Cross-Platform**: Supports Windows, Linux, and macOS
- **Job Management**: Integrated solver execution and monitoring
- **Model Analysis**: Real-time mesh quality analysis and statistics

## Screenshots

*TODO: Add screenshots of the application*

## Prerequisites

### Required Dependencies

- **CMake 3.16+**
- **C++17 compatible compiler** (GCC 7+, Clang 5+, MSVC 2017+)
- **OpenGL 3.3+** support
- **Git** (for cloning submodules)

### System Dependencies

#### Windows
```bash
# Install via vcpkg (recommended)
vcpkg install glfw3 glew glm

# Or download manually:
# - GLFW3: https://www.glfw.org/download.html
# - GLEW: http://glew.sourceforge.net/
# - GLM: https://github.com/g-truc/glm
```

#### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install cmake build-essential git
sudo apt install libglfw3-dev libglew-dev libglm-dev
sudo apt install libgtk-3-dev  # For file dialogs
```

#### Linux (CentOS/RHEL/Fedora)
```bash
sudo dnf install cmake gcc-c++ git
sudo dnf install glfw-devel glew-devel glm-devel
sudo dnf install gtk3-devel  # For file dialogs
```

#### macOS
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake glfw glew glm
```

## Building from Source

### Quick Start

```bash
# Clone the repository
git clone https://github.com/MC912/OpenGl_GUI_For_OpenRadioss.git
cd OpenGl_GUI_For_OpenRadioss

# Initialize submodules (for Dear ImGui and other dependencies)
git submodule update --init --recursive

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build . --config Release

# Run the application
./OpenRadiossGUI  # Linux/macOS
# or
OpenRadiossGUI.exe  # Windows
```

### Detailed Build Instructions

#### 1. Clone and Setup
```bash
git clone https://github.com/MC912/OpenGl_GUI_For_OpenRadioss.git
cd OpenGl_GUI_For_OpenRadioss
git submodule update --init --recursive
```

#### 2. Configure Build Options
```bash
mkdir build && cd build

# Basic configuration
cmake ..

# With custom options
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON \
    -DCMAKE_INSTALL_PREFIX=/usr/local
```

#### 3. Build
```bash
# Build in release mode (recommended)
cmake --build . --config Release

# Build in debug mode (for development)
cmake --build . --config Debug

# Parallel build (faster)
cmake --build . --config Release --parallel 4
```

#### 4. Install (Optional)
```bash
sudo cmake --install .
```

### Platform-Specific Notes

#### Windows with Visual Studio
```bash
# Generate Visual Studio solution
cmake .. -G "Visual Studio 16 2019" -A x64

# Open the solution
start OpenRadiossGUI.sln

# Or build from command line
cmake --build . --config Release
```

#### Linux with Different Compilers
```bash
# Use GCC
cmake .. -DCMAKE_CXX_COMPILER=g++

# Use Clang
cmake .. -DCMAKE_CXX_COMPILER=clang++
```

#### macOS with Xcode
```bash
# Generate Xcode project
cmake .. -G Xcode

# Open project
open OpenRadiossGUI.xcodeproj
```

## Project Structure

```
OpenGl_GUI_For_OpenRadioss/
├── CMakeLists.txt              # Main build configuration
├── README.md                   # This file
├── LICENSE                     # License file
├── include/                    # Header files
│   └── radfilereader.h        # RAD file parser interface
├── src/                       # Source files
│   ├── main.cpp              # Main application entry point
│   └── radfilereader.cpp     # RAD file parser implementation
├── third_party/              # Third-party dependencies
│   ├── imgui/                # Dear ImGui (submodule)
│   ├── nativefiledialog/     # Native file dialog (submodule)
│   └── glm/                  # GLM math library (optional)
├── assets/                   # Application assets
│   ├── shaders/             # GLSL shader files
│   ├── fonts/               # Font files
│   └── icons/               # Application icons
├── examples/                 # Example RAD files
│   ├── simple_beam.rad
│   ├── crash_test.rad
│   └── tensile_test.rad
├── docs/                    # Documentation
│   ├── user_guide.md
│   ├── developer_guide.md
│   └── api_reference.md
└── tests/                   # Unit tests (optional)
    ├── test_radreader.cpp
    └── test_geometry.cpp
```

## Usage

### Loading a Model
1. **File → Open...** or **Ctrl+O**
2. Select a `.rad` file from your OpenRadioss installation or examples
3. The model will be automatically loaded and visualized

### Navigation
- **Mouse Left Drag**: Orbit camera around model
- **Mouse Wheel**: Zoom in/out
- **Right Panel**: Adjust rendering options and view model statistics

### Rendering Options
- **Show Nodes**: Toggle node visibility
- **Show Elements**: Toggle element visibility  
- **Wireframe Mode**: Toggle between solid and wireframe rendering
- **Background Color**: Customize viewport background

### Supported File Formats
- **RAD files** (`.rad`): Native OpenRadioss format
- **Keyword files** (`.key`, `.k`): LS-DYNA format (limited support)

## OpenRadioss Integration

### Solver Execution
The GUI provides integrated solver execution capabilities:

1. **Tools → Run OpenRadioss Starter**: Execute the starter phase
2. **Tools → Run OpenRadioss Engine**: Execute the engine phase
3. **Monitor Progress**: Real-time output monitoring

### Environment Setup
Ensure OpenRadioss is properly installed and accessible:

#### Linux/macOS
```bash
export OPENRADIOSS_PATH=/path/to/openradioss
export RAD_CFG_PATH=$OPENRADIOSS_PATH/hm_cfg_files
export PATH=$OPENRADIOSS_PATH/exec:$PATH
```

#### Windows
```cmd
set OPENRADIOSS_PATH=C:\OpenRadioss
set RAD_CFG_PATH=%OPENRADIOSS_PATH%\hm_cfg_files
set PATH=%OPENRADIOSS_PATH%\exec;%PATH%
```

## Development

### Building for Development
```bash
# Debug build with symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON

# Enable additional warnings
cmake .. -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic"

# Build and run tests
cmake --build . --target test
```

### Code Style
- **C++ Standard**: C++17
- **Naming Convention**: `snake_case` for variables, `PascalCase` for classes
- **Indentation**: 4 spaces, no tabs
- **Line Length**: Maximum 100 characters

### Adding New Features
1. Create feature branch: `git checkout -b feature/new-feature`
2. Implement changes in appropriate files
3. Add tests if applicable
4. Update documentation
5. Submit pull request

### Debugging
```bash
# Build with debug symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Run with GDB (Linux)
gdb ./OpenRadiossGUI

# Run with LLDB (macOS)
lldb ./OpenRadiossGUI

# Enable OpenGL debugging
export MESA_DEBUG=1  # Linux with Mesa
```

## Troubleshooting

### Common Build Issues

#### GLEW Not Found
```bash
# Ubuntu/Debian
sudo apt install libglew-dev

# CentOS/RHEL
sudo dnf install glew-devel

# macOS
brew install glew
```

#### GLFW Not Found
```bash
# Ubuntu/Debian
sudo apt install libglfw3-dev

# CentOS/RHEL
sudo dnf install glfw-devel

# macOS
brew install glfw
```

#### CMake Version Too Old
```bash
# Install newer CMake from official website
wget https://github.com/Kitware/CMake/releases/download/v3.21.0/cmake-3.21.0.tar.gz
tar -xzf cmake-3.21.0.tar.gz
cd cmake-3.21.0
./bootstrap && make && sudo make install
```

### Runtime Issues

#### OpenGL Context Creation Failed
- Ensure your graphics drivers are up to date
- Check OpenGL version: `glxinfo | grep "OpenGL version"` (Linux)
- Try running with software rendering: `LIBGL_ALWAYS_SOFTWARE=1 ./OpenRadiossGUI`

#### File Loading Errors
- Verify RAD file format compatibility
- Check file permissions
- Ensure file path contains no special characters

#### Performance Issues
- Update graphics drivers
- Close other OpenGL applications
- Reduce model complexity for real-time interaction
- Enable/disable multisampling in render options

## Contributing

We welcome contributions! Please read our contributing guidelines:

1. **Fork** the repository
2. **Create** a feature branch
3. **Commit** your changes with clear messages
4. **Add tests** for new functionality
5. **Update documentation** as needed
6. **Submit** a pull request

### Bug Reports
Please include:
- Operating system and version
- Compiler and version
- CMake version
- Steps to reproduce
- Expected vs actual behavior
- Relevant log output

### Feature Requests
- Describe the feature clearly
- Explain the use case
- Provide mockups if applicable
- Consider implementation complexity

## License

This project is licensed under the GNU Affero General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **OpenRadioss Team**: For the excellent open-source FEA solver
- **Dear ImGui**: For the immediate mode GUI framework
- **GLFW**: For cross-platform windowing
- **GLM**: For mathematics library
- **Community Contributors**: For bug reports, feature requests, and code contributions

## Related Projects

- [OpenRadioss](https://github.com/OpenRadioss/OpenRadioss): Main FEA solver
- [OpenRadioss Tools](https://github.com/OpenRadioss/Tools): Official tools collection
- [Gmsh](https://gmsh.info/): Open-source mesh generator
- [ParaView](https://www.paraview.org/): Open-source post-processor

## Support

- **Issues**: [GitHub Issues](https://github.com/MC912/OpenGl_GUI_For_OpenRadioss/issues)
- **Discussions**: [GitHub Discussions](https://github.com/MC912/OpenGl_GUI_For_OpenRadioss/discussions)
- **Wiki**: [Project Wiki](https://github.com/MC912/OpenGl_GUI_For_OpenRadioss/wiki)
- **OpenRadioss Community**: [OpenRadioss Discussions](https://github.com/orgs/OpenRadioss/discussions)

