#!/bin/bash

# OpenRadioss GUI Build Script
# This script automates the build process and checks for dependencies

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print colored output
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

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Detect operating system
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
        echo "windows"
    else
        echo "unknown"
    fi
}

# Check dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    local missing_deps=()
    
    # Check CMake
    if ! command_exists cmake; then
        missing_deps+=("cmake")
    else
        local cmake_version=$(cmake --version | head -n1 | cut -d' ' -f3)
        print_success "CMake found: $cmake_version"
    fi
    
    # Check C++ compiler
    if command_exists g++; then
        local gcc_version=$(g++ --version | head -n1)
        print_success "GCC found: $gcc_version"
    elif command_exists clang++; then
        local clang_version=$(clang++ --version | head -n1)
        print_success "Clang found: $clang_version"
    else
        missing_deps+=("g++ or clang++")
    fi
    
    # Check Git
    if ! command_exists git; then
        missing_deps+=("git")
    else
        local git_version=$(git --version)
        print_success "Git found: $git_version"
    fi
    
    # Check system-specific dependencies
    local os=$(detect_os)
    case $os in
        "linux")
            check_linux_deps
            ;;
        "macos")
            check_macos_deps
            ;;
        "windows")
            check_windows_deps
            ;;
        *)
            print_warning "Unknown operating system, proceeding anyway..."
            ;;
    esac
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_status "Please install missing dependencies and try again."
        return 1
    fi
    
    return 0
}

# Check Linux-specific dependencies
check_linux_deps() {
    print_status "Checking Linux dependencies..."
    
    # Check for development packages
    local packages=("libglfw3-dev" "libglew-dev" "libglm-dev" "libgtk-3-dev")
    local missing_packages=()
    
    for package in "${packages[@]}"; do
        if ! dpkg -l | grep -q "^ii  $package "; then
            if ! rpm -q "$package" >/dev/null 2>&1; then
                missing_packages+=("$package")
            fi
        fi
    done
    
    if [ ${#missing_packages[@]} -ne 0 ]; then
        print_warning "Some packages might be missing: ${missing_packages[*]}"
        print_status "Install with:"
        print_status "  Ubuntu/Debian: sudo apt install ${missing_packages[*]}"
        print_status "  CentOS/RHEL: sudo dnf install ${missing_packages[*]//dev/devel}"
    fi
}

# Check macOS-specific dependencies
check_macos_deps() {
    print_status "Checking macOS dependencies..."
    
    if ! command_exists brew; then
        print_warning "Homebrew not found. Install from https://brew.sh/"
        print_status "Then run: brew install cmake glfw glew glm"
    else
        print_success "Homebrew found"
        
        local packages=("cmake" "glfw" "glew" "glm")
        for package in "${packages[@]}"; do
            if brew list | grep -q "^$package$"; then
                print_success "$package installed"
            else
                print_warning "$package not installed. Run: brew install $package"
            fi
        done
    fi
}

# Check Windows-specific dependencies
check_windows_deps() {
    print_status "Checking Windows dependencies..."
    print_status "For Windows, ensure you have:"
    print_status "  - Visual Studio 2017+ or MinGW-w64"
    print_status "  - vcpkg or manually installed libraries"
    print_status "  - Run: vcpkg install glfw3 glew glm"
}

# Initialize submodules
init_submodules() {
    print_status "Initializing submodules..."
    
    if [ ! -d ".git" ]; then
        print_error "Not in a git repository. Please clone the project properly."
        return 1
    fi
    
    git submodule update --init --recursive
    print_success "Submodules initialized"
}

# Create build directory
setup_build_dir() {
    local build_type=${1:-Release}
    local build_dir="build"
    
    print_status "Setting up build directory..."
    
    # Remove old build directory if requested
    if [ "$CLEAN_BUILD" = "true" ] && [ -d "$build_dir" ]; then
        print_status "Removing old build directory..."
        rm -rf "$build_dir"
    fi
    
    mkdir -p "$build_dir"
    cd "$build_dir"
    
    print_status "Configuring CMake (Build Type: $build_type)..."
    
    # CMake configuration
    local cmake_args=(
        "-DCMAKE_BUILD_TYPE=$build_type"
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    )
    
    # Add additional arguments if specified
    if [ -n "$CMAKE_ARGS" ]; then
        cmake_args+=($CMAKE_ARGS)
    fi
    
    cmake "${cmake_args[@]}" ..
    
    print_success "CMake configuration complete"
}

# Build the project
build_project() {
    local parallel_jobs=${1:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}
    
    print_status "Building project with $parallel_jobs parallel jobs..."
    
    cmake --build . --parallel "$parallel_jobs"
    
    print_success "Build complete"
}

# Run basic tests
run_tests() {
    print_status "Running basic tests..."
    
    # Check if executable exists
    local exe_name="OpenRadiossGUI"
    if [[ $(detect_os) == "windows" ]]; then
        exe_name="${exe_name}.exe"
    fi
    
    if [ -f "$exe_name" ]; then
        print_success "Executable created successfully: $exe_name"
        
        # Try to get version info (if supported)
        if ./"$exe_name" --version >/dev/null 2>&1; then
            local version=$(./"$exe_name" --version 2>/dev/null)
            print_success "Version: $version"
        fi
    else
        print_error "Executable not found: $exe_name"
        return 1
    fi
    
    # Run unit tests if available
    if [ -f "test_runner" ]; then
        print_status "Running unit tests..."
        ./test_runner
        print_success "Unit tests passed"
    fi
}

# Print usage information
print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -c, --clean         Clean build directory before building"
    echo "  -d, --debug         Build in Debug mode (default: Release)"
    echo "  -j, --jobs N        Number of parallel build jobs (default: auto)"
    echo "  -t, --test          Run tests after building"
    echo "  --check-only        Only check dependencies, don't build"
    echo "  --cmake-args ARGS   Additional CMake arguments"
    echo ""
    echo "Examples:"
    echo "  $0                  # Build in Release mode"
    echo "  $0 -d               # Build in Debug mode"
    echo "  $0 -c -j 8          # Clean build with 8 parallel jobs"
    echo "  $0 --check-only     # Only check dependencies"
}

# Main function
main() {
    local build_type="Release"
    local parallel_jobs=""
    local run_tests=false
    local check_only=false
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                print_usage
                exit 0
                ;;
            -c|--clean)
                CLEAN_BUILD="true"
                shift
                ;;
            -d|--debug)
                build_type="Debug"
                shift
                ;;
            -j|--jobs)
                parallel_jobs="$2"
                shift 2
                ;;
            -t|--test)
                run_tests=true
                shift
                ;;
            --check-only)
                check_only=true
                shift
                ;;
            --cmake-args)
                CMAKE_ARGS="$2"
                shift 2
                ;;
            *)
                print_error "Unknown option: $1"
                print_usage
                exit 1
                ;;
        esac
    done
    
    print_status "OpenRadioss GUI Build Script"
    print_status "Operating System: $(detect_os)"
    
    # Check dependencies
    if ! check_dependencies; then
        exit 1
    fi
    
    if [ "$check_only" = "true" ]; then
        print_success "Dependency check complete"
        exit 0
    fi
    
    # Initialize submodules
    if ! init_submodules; then
        exit 1
    fi
    
    # Set up build directory
    if ! setup_build_dir "$build_type"; then
        exit 1
    fi
    
    # Build project
    if ! build_project "$parallel_jobs"; then
        exit 1
    fi
    
    # Run tests if requested
    if [ "$run_tests" = "true" ]; then
        if ! run_tests; then
            print_warning "Tests failed, but build succeeded"
        fi
    fi
    
    print_success "Build completed successfully!"
    print_status "Executable location: $(pwd)/OpenRadiossGUI"
    print_status "To run: ./OpenRadiossGUI"
}

# Run main function with all arguments
main "$@"