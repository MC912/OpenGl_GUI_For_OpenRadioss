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
