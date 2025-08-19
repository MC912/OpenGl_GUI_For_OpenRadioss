cmake_minimum_required(VERSION 3.16)
project(OpenRadiossGUI VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(USE_SYSTEM_LIBS "Use system libraries instead of bundled ones" ON)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX -DWIN32_LEAN_AND_MEAN)
    # Enable parallel compilation
    add_compile_options(/MP)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# Find required packages
find_package(OpenGL REQUIRED)

# Try to find GLFW3
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLFW3 QUIET glfw3)
    endif()
endif()

# Try to find GLEW
find_package(GLEW QUIET)
if(NOT GLEW_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLEW QUIET glew)
    endif()
endif()

# Try to find GLM
find_package(glm QUIET)

# Set up paths - ensure case sensitivity
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# Create directories if they don't exist
foreach(dir ${INCLUDE_DIR} ${SRC_DIR} ${THIRD_PARTY_DIR})
    if(NOT EXISTS ${dir})
        file(MAKE_DIRECTORY ${dir})
        message(STATUS "Created directory: ${dir}")
    endif()
endforeach()

# Source files
set(MAIN_SOURCES)

# Find main.cpp
if(EXISTS ${SRC_DIR}/main.cpp)
    list(APPEND MAIN_SOURCES ${SRC_DIR}/main.cpp)
    message(STATUS "Found main.cpp in src/")
elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
    list(APPEND MAIN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
    message(STATUS "Found main.cpp in project root")
else()
    message(FATAL_ERROR "main.cpp not found in ${SRC_DIR}/ or project root")
endif()

# Find radfilereader.cpp
if(EXISTS ${SRC_DIR}/radfilereader.cpp)
    list(APPEND MAIN_SOURCES ${SRC_DIR}/radfilereader.cpp)
    message(STATUS "Found radfilereader.cpp in src/")
elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/radfilereader.cpp)
    list(APPEND MAIN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/radfilereader.cpp)
    message(STATUS "Found radfilereader.cpp in project root")
else()
    message(WARNING "radfilereader.cpp not found - will create placeholder")
    # Create a minimal placeholder if missing
    set(PLACEHOLDER_RAD_CPP ${CMAKE_BINARY_DIR}/radfilereader_placeholder.cpp)
    file(WRITE ${PLACEHOLDER_RAD_CPP} 
        "#include \"radfilereader.h\"\n"
        "// Placeholder implementation\n"
        "namespace OpenRadiossGUI {\n"
        "RadFileReader::RadFileReader() : isValid_(false) {}\n"
        "RadFileReader::~RadFileReader() {}\n"
        "bool RadFileReader::loadFile(const std::string&) { return false; }\n"
        "bool RadFileReader::saveFile(const std::string&) const { return false; }\n"
        "void RadFileReader::clear() {}\n"
        "const Node* RadFileReader::findNode(int) const { return nullptr; }\n"
        "const Element* RadFileReader::findElement(int) const { return nullptr; }\n"
        "std::pair<glm::vec3, glm::vec3> RadFileReader::getBoundingBox() const {\n"
        "  return std::make_pair(glm::vec3(0.0f), glm::vec3(0.0f));\n"
        "}\n"
        "}\n"
    )
    list(APPEND MAIN_SOURCES ${PLACEHOLDER_RAD_CPP})
endif()

# Header files
set(MAIN_HEADERS)
if(EXISTS ${INCLUDE_DIR}/radfilereader.h)
    list(APPEND MAIN_HEADERS ${INCLUDE_DIR}/radfilereader.h)
    message(STATUS "Found radfilereader.h in include/")
elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/radfilereader.h)
    list(APPEND MAIN_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/radfilereader.h)
    message(STATUS "Found radfilereader.h in project root")
else()
    message(WARNING "radfilereader.h not found - will create placeholder")
    # Create a minimal placeholder header
    set(PLACEHOLDER_RAD_H ${CMAKE_BINARY_DIR}/radfilereader_placeholder.h)
    file(WRITE ${PLACEHOLDER_RAD_H}
        "#pragma once\n"
        "#include <string>\n"
        "#include <vector>\n"
        "#include <glm/glm.hpp>\n"
        "namespace OpenRadiossGUI {\n"
        "struct Node { int id; glm::vec3 position; };\n"
        "struct Element { enum Type { UNKNOWN=0, TRIA3=3, QUAD4=4, TETRA4=10, HEXA8=12 };\n"
        "  int id; Type type; int materialId, propertyId; std::vector<int> nodeIds; };\n"
        "class RadFileReader {\n"
        "public:\n"
        "  RadFileReader(); ~RadFileReader();\n"
        "  bool loadFile(const std::string& filename);\n"
        "  bool saveFile(const std::string& filename) const;\n"
        "  void clear();\n"
        "  const std::vector<Node>& getNodes() const { return nodes_; }\n"
        "  const std::vector<Element>& getElements() const { return elements_; }\n"
        "  std::string getTitle() const { return title_; }\n"
        "  bool isValid() const { return isValid_; }\n"
        "  std::string getLastError() const { return lastError_; }\n"
        "  size_t getNodeCount() const { return nodes_.size(); }\n"
        "  size_t getElementCount() const { return elements_.size(); }\n"
        "  size_t getMaterialCount() const { return 0; }\n"
        "  const Node* findNode(int id) const;\n"
        "  const Element* findElement(int id) const;\n"
        "  std::pair<glm::vec3, glm::vec3> getBoundingBox() const;\n"
        "private:\n"
        "  std::vector<Node> nodes_; std::vector<Element> elements_;\n"
        "  std::string title_, lastError_; bool isValid_;\n"
        "};\n"
        "}\n"
    )
    list(APPEND MAIN_HEADERS ${PLACEHOLDER_RAD_H})
endif()

# Dear ImGui sources
set(IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)
set(IMGUI_SOURCES)

if(EXISTS ${IMGUI_DIR}/imgui.cpp)
    message(STATUS "Found Dear ImGui in ${IMGUI_DIR}")
    list(APPEND IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
    )
    
    # Check for backends
    if(EXISTS ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp)
        list(APPEND IMGUI_SOURCES 
            ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
            ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        )
        message(STATUS "Found ImGui GLFW and OpenGL3 backends")
    else()
        message(WARNING "ImGui backends not found - GUI may not work")
    endif()
else()
    message(WARNING "Dear ImGui not found in ${IMGUI_DIR}")
    message(STATUS "Run: git submodule update --init --recursive")
endif()

# Native File Dialog sources
set(NFD_DIR ${THIRD_PARTY_DIR}/nativefiledialog)
set(NFD_SOURCES)
set(NFD_LIBRARIES)

if(EXISTS ${NFD_DIR}/src/nfd_common.c)
    message(STATUS "Found Native File Dialog in ${NFD_DIR}")
    list(APPEND NFD_SOURCES ${NFD_DIR}/src/nfd_common.c)
    
    # Platform-specific sources
    if(WIN32)
        if(EXISTS ${NFD_DIR}/src/nfd_win.cpp)
            list(APPEND NFD_SOURCES ${NFD_DIR}/src/nfd_win.cpp)
        endif()
        set(NFD_LIBRARIES ole32 uuid)
    elseif(APPLE)
        if(EXISTS ${NFD_DIR}/src/nfd_cocoa.m)
            list(APPEND NFD_SOURCES ${NFD_DIR}/src/nfd_cocoa.m)
        endif()
        set(NFD_LIBRARIES "-framework AppKit")
    else()
        if(EXISTS ${NFD_DIR}/src/nfd_gtk.c)
            list(APPEND NFD_SOURCES ${NFD_DIR}/src/nfd_gtk.c)
        endif()
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(GTK3 QUIET gtk+-3.0)
            if(GTK3_FOUND)
                set(NFD_LIBRARIES ${GTK3_LIBRARIES})
                message(STATUS "Found GTK3 for file dialogs")
            else()
                message(WARNING "GTK3 not found - file dialogs may not work on Linux")
            endif()
        endif()
    endif()
else()
    message(WARNING "Native File Dialog not found - file dialogs will use fallback")
endif()

# Create the executable
add_executable(${PROJECT_NAME}
    ${MAIN_SOURCES}
    ${MAIN_HEADERS}
    ${IMGUI_SOURCES}
    ${NFD_SOURCES}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}  # For placeholder files
    ${INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIRS}
)

# Add ImGui includes if available
if(EXISTS ${IMGUI_DIR}/imgui.h)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_IMGUI)
endif()

# Add NFD includes if available
if(EXISTS ${NFD_DIR}/src/include/nfd.h)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${NFD_DIR}/src/include
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_NFD)
endif()

# Link OpenGL
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

# Link GLFW
if(glfw3_FOUND)
    target_link_libraries(${PROJECT_NAME} glfw)
    message(STATUS "Linking with glfw3 (found via CMake)")
elseif(GLFW3_FOUND)
    target_link_libraries(${PROJECT_NAME} ${GLFW3_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW3_INCLUDE_DIRS})
    target_compile_options(${PROJECT_NAME} PRIVATE ${GLFW3_CFLAGS_OTHER})
    message(STATUS "Linking with GLFW3 (found via pkg-config)")
else()
    # Try to find system GLFW
    find_library(GLFW_LIBRARY NAMES glfw glfw3)
    if(GLFW_LIBRARY)
        target_link_libraries(${PROJECT_NAME} ${GLFW_LIBRARY})
        message(STATUS "Linking with GLFW (found system library)")
    else()
        message(FATAL_ERROR "GLFW not found. Please install GLFW3 development libraries.")
    endif()
endif()

# Link GLEW
if(GLEW_FOUND)
    if(TARGET GLEW::GLEW)
        target_link_libraries(${PROJECT_NAME} GLEW::GLEW)
        message(STATUS "Linking with GLEW::GLEW (CMake target)")
    else()
        target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})
        target_include_directories(${PROJECT_NAME} PRIVATE ${GLEW_INCLUDE_DIRS})
        message(STATUS "Linking with GLEW (libraries)")
    endif()
else()
    # Try to find system GLEW
    find_library(GLEW_LIBRARY NAMES GLEW glew32 glew)
    if(GLEW_LIBRARY)
        target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARY})
        message(STATUS "Linking with GLEW (found system library)")
    else()
        message(FATAL_ERROR "GLEW not found. Please install GLEW development libraries.")
    endif()
endif()

# Link GLM (header-only)
if(glm_FOUND)
    if(TARGET glm::glm)
        target_link_libraries(${PROJECT_NAME} glm::glm)
        message(STATUS "Using GLM (CMake target)")
    endif()
else()
    # Try to find GLM headers
    find_path(GLM_INCLUDE_DIR glm/glm.hpp 
        PATHS 
            ${THIRD_PARTY_DIR}/glm
            /usr/include
            /usr/local/include
            /opt/homebrew/include
    )
    if(GLM_INCLUDE_DIR)
        target_include_directories(${PROJECT_NAME} PRIVATE ${GLM_INCLUDE_DIR})
        message(STATUS "Using GLM from ${GLM_INCLUDE_DIR}")
    else()
        message(FATAL_ERROR "GLM not found. Please install GLM or run 'git submodule update --init --recursive'")
    endif()
endif()

# Link NFD libraries
if(NFD_LIBRARIES)
    target_link_libraries(${PROJECT_NAME} ${NFD_LIBRARIES})
endif()

# Add GTK3 for Linux file dialogs
if(GTK3_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${GTK3_INCLUDE_DIRS})
    target_compile_options(${PROJECT_NAME} PRIVATE ${GTK3_CFLAGS_OTHER})
    link_directories(${GTK3_LIBRARY_DIRS})
endif()

# Platform-specific settings
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
    
elseif(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
    )
    
else()
    # Linux-specific settings
    target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS} pthread)
endif()

# Compiler definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    GLFW_INCLUDE_NONE  # Don't include OpenGL headers from GLFW
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Installation
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

# Install assets if they exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets
        DESTINATION share/${PROJECT_NAME}
    )
endif()

# Only add tests subdirectory if it exists and BUILD_TESTS is ON
if(BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
    message(STATUS "Tests enabled")
endif()

# Package configuration
set(CPACK_PACKAGE_NAME "OpenRadioss GUI")
set(CPACK_PACKAGE_VENDOR "OpenRadioss Community")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenGL-based GUI for OpenRadioss")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

# Platform-specific packaging
if(WIN32)
    set(CPACK_GENERATOR "ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)

# Custom targets
add_custom_target(run
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running ${PROJECT_NAME}"
)

# Create a target to copy example files
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
    add_custom_target(copy_examples ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/examples
        ${CMAKE_CURRENT_BINARY_DIR}/examples
        COMMENT "Copying example files"
    )
    add_dependencies(${PROJECT_NAME} copy_examples)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "=== OpenRadioss GUI Configuration Summary ===")
message(STATUS "  Project Name:     ${PROJECT_NAME}")
message(STATUS "  Version:          ${PROJECT_VERSION}")
message(STATUS "  Build Type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard:     ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Install Prefix:   ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "=== Dependencies Status ===")
message(STATUS "  OpenGL:           ${OPENGL_FOUND}")
if(glfw3_FOUND)
    message(STATUS "  GLFW:             Found (CMake)")
elseif(GLFW3_FOUND)
    message(STATUS "  GLFW:             Found (pkg-config)")
else()
    message(STATUS "  GLFW:             Found (system)")
endif()

if(GLEW_FOUND)
    message(STATUS "  GLEW:             Found (CMake)")
else()
    message(STATUS "  GLEW:             Found (system)")
endif()

if(glm_FOUND)
    message(STATUS "  GLM:              Found (CMake)")
else()
    message(STATUS "  GLM:              Found (headers)")
endif()

if(EXISTS ${IMGUI_DIR}/imgui.cpp)
    message(STATUS "  Dear ImGui:       Available")
else()
    message(STATUS "  Dear ImGui:       Missing (run git submodule update --init)")
endif()

if(EXISTS ${NFD_DIR}/src/nfd_common.c)
    message(STATUS "  File Dialog:      Available (Native)")
else()
    message(STATUS "  File Dialog:      Fallback mode")
endif()

if(GTK3_FOUND)
    message(STATUS "  GTK3 (Linux):     Found")
elseif(UNIX AND NOT APPLE)
    message(STATUS "  GTK3 (Linux):     Not found")
endif()

message(STATUS "")
message(STATUS "=== Source Files ===")
foreach(source ${MAIN_SOURCES})
    get_filename_component(source_name ${source} NAME)
    message(STATUS "  ${source_name}")
endforeach()

if(IMGUI_SOURCES)
    message(STATUS "  Dear ImGui:       ${IMGUI_DIR}")
endif()

if(NFD_SOURCES)
    message(STATUS "  File Dialog:      ${NFD_DIR}")
endif()

message(STATUS "")
message(STATUS "=== Build Instructions ===")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake ..")
message(STATUS "  cmake --build . --config ${CMAKE_BUILD_TYPE}")
message(STATUS "  # Run with: ./${PROJECT_NAME}")
message(STATUS "")

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
    message(STATUS "=== Example Files ===")
    file(GLOB EXAMPLE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.rad")
    foreach(example ${EXAMPLE_FILES})
        get_filename_component(example_name ${example} NAME)
        message(STATUS "  ${example_name}")
    endforeach()
    message(STATUS "  # Test with: ./${PROJECT_NAME} examples/test.rad")
    message(STATUS "")
endif()

message(STATUS "=== Keyboard Shortcuts (when running) ===")
message(STATUS "  Ctrl+O        Open file")
message(STATUS "  R             Reset camera")
message(STATUS "  1             Toggle nodes")
message(STATUS "  2             Toggle elements") 
message(STATUS "  3             Toggle wireframe")
message(STATUS "  4             Toggle axes")
message(STATUS "  F1            About dialog")
message(STATUS "  ESC           Exit")
message(STATUS "")

# Warning messages for missing components
if(NOT EXISTS ${IMGUI_DIR}/imgui.cpp)
    message(WARNING "Dear ImGui not found! Run: git submodule update --init --recursive")
endif()

if(NOT EXISTS ${NFD_DIR}/src/nfd_common.c)
    message(WARNING "Native File Dialog not found! File operations will use fallback.")
endif()

if(UNIX AND NOT APPLE AND NOT GTK3_FOUND)
    message(WARNING "GTK3 not found on Linux! Install with: sudo apt install libgtk-3-dev")
endif()

message(STATUS "==========================================")

# Create a simple test to verify the build works
enable_testing()
add_test(NAME build_test
    COMMAND ${PROJECT_NAME} --version
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Create run scripts for convenience
if(UNIX)
    # Create a simple run script
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/run.sh
        "#!/bin/bash\n"
        "cd $(dirname $0)\n"
        "if [ ! -f ${PROJECT_NAME} ]; then\n"
        "    echo 'Build first with: cmake --build .'\n"
        "    exit 1\n"
        "fi\n"
        "\n"
        "echo 'Starting OpenRadioss GUI...'\n"
        "./${PROJECT_NAME} \"$@\"\n"
    )
    
    # Make it executable
    file(COPY ${CMAKE_CURRENT_BINARY_DIR}/run.sh
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    )
endif()

if(WIN32)
    # Create a simple batch file
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/run.bat
        "@echo off\n"
        "if not exist ${PROJECT_NAME}.exe (\n"
        "    echo Build first with: cmake --build . --config Release\n"
        "    pause\n"
        "    exit /b 1\n"
        ")\n"
        "\n"
        "echo Starting OpenRadioss GUI...\n"
        "${PROJECT_NAME}.exe %*\n"
    )
endif()