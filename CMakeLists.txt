cmake_minimum_required(VERSION 3.16)
project(OpenRadiossGUI VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Try to find system packages first
find_package(PkgConfig QUIET)

# GLEW
find_package(GLEW QUIET)
if(NOT GLEW_FOUND)
    message(STATUS "GLEW not found, will use fallback")
endif()

# Set up include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# Source files
set(SOURCES
    src/main.cpp
    src/radfilereader.cpp
)

# Header files
set(HEADERS
    include/radfilereader.h
)

# Third-party libraries (if not found system-wide)
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# ImGui sources
set(IMGUI_SOURCES
    ${THIRD_PARTY_DIR}/imgui/imgui.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_demo.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_draw.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_tables.cpp
    ${THIRD_PARTY_DIR}/imgui/imgui_widgets.cpp
    ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_glfw.cpp
    ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)

# Native File Dialog sources
set(NFD_SOURCES
    ${THIRD_PARTY_DIR}/nativefiledialog/src/nfd_common.c
)

# Platform-specific NFD sources
if(WIN32)
    list(APPEND NFD_SOURCES ${THIRD_PARTY_DIR}/nativefiledialog/src/nfd_win.cpp)
    set(NFD_LIBRARIES ole32 uuid)
elseif(APPLE)
    list(APPEND NFD_SOURCES ${THIRD_PARTY_DIR}/nativefiledialog/src/nfd_cocoa.m)
    set(NFD_LIBRARIES "-framework AppKit")
else()
    list(APPEND NFD_SOURCES ${THIRD_PARTY_DIR}/nativefiledialog/src/nfd_gtk.c)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    set(NFD_LIBRARIES ${GTK3_LIBRARIES})
    include_directories(${GTK3_INCLUDE_DIRS})
    link_directories(${GTK3_LIBRARY_DIRS})
    add_definitions(${GTK3_CFLAGS_OTHER})
endif()

# Create executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${IMGUI_SOURCES}
    ${NFD_SOURCES}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${THIRD_PARTY_DIR}/imgui
    ${THIRD_PARTY_DIR}/imgui/backends
    ${THIRD_PARTY_DIR}/nativefiledialog/src/include
    ${OPENGL_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    glfw
    ${NFD_LIBRARIES}
)

# Link GLEW
if(GLEW_FOUND)
    target_link_libraries(${PROJECT_NAME} GLEW::GLEW)
else()
    # Fallback: link system GLEW or provide alternative
    if(WIN32)
        find_library(GLEW_LIBRARY NAMES glew32 glew PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib)
        if(GLEW_LIBRARY)
            target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARY})
        else()
            message(FATAL_ERROR "GLEW library not found. Please install GLEW or provide library path.")
        endif()
    else()
        target_link_libraries(${PROJECT_NAME} GLEW)
    endif()
endif()

# GLM (header-only)
if(TARGET glm::glm)
    target_link_libraries(${PROJECT_NAME} glm::glm)
else()
    # GLM is header-only, just need include path
    find_path(GLM_INCLUDE_DIR glm/glm.hpp PATHS ${THIRD_PARTY_DIR}/glm)
    if(GLM_INCLUDE_DIR)
        target_include_directories(${PROJECT_NAME} PRIVATE ${GLM_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "GLM not found. Please install GLM or provide include path.")
    endif()
endif()

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # Copy DLLs to output directory in Debug mode
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Add custom post-build step to copy required DLLs
        # This would need to be customized based on your DLL locations
    endif()
    
elseif(APPLE)
    # macOS-specific settings
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
    )
    
else()
    # Linux-specific settings
    # Link additional libraries if needed
    target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})
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

# Package configuration (optional)
set(CPACK_PACKAGE_NAME "OpenRadioss GUI")
set(CPACK_PACKAGE_VENDOR "OpenRadioss Community")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenGL-based GUI for OpenRadioss")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "OpenRadiossGUI")

# Platform-specific packaging
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "OpenRadioss Community")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libglfw3-dev, libglew-dev, libglm-dev")
endif()

include(CPack)

# Custom targets for development
add_custom_target(run
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running ${PROJECT_NAME}"
)

# Documentation target (if Doxygen is available)
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)
    set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
    set(DOXYGEN_PROJECT_VERSION ${PROJECT_VERSION})
    
    doxygen_add_docs(docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        COMMENT "Generating documentation"
    )
endif()

# Testing (optional)
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest QUIET)
    if(GTest_FOUND)
        add_subdirectory(tests)
    else()
        message(WARNING "Google Test not found, tests will not be built")
    endif()
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Project Name:     ${PROJECT_NAME}")
message(STATUS "  Version:          ${PROJECT_VERSION}")
message(STATUS "  Build Type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard:     ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Install Prefix:   ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  OpenGL:           ${OPENGL_FOUND}")
message(STATUS "  GLFW:             ${glfw3_FOUND}")
message(STATUS "  GLEW:             ${GLEW_FOUND}")
message(STATUS "  GLM:              Found")
message(STATUS "")

# Warning for missing dependencies
if(NOT OPENGL_FOUND)
    message(WARNING "OpenGL not found! The application may not build correctly.")
endif()

if(NOT glfw3_FOUND)
    message(WARNING "GLFW3 not found! The application may not build correctly.")
endif()

# Build instructions
message(STATUS "Build Instructions:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake ..")
message(STATUS "  cmake --build . --config ${CMAKE_BUILD_TYPE}")
message(STATUS "  # Run with: ./${PROJECT_NAME} (or cmake --build . --target run)")
message(STATUS "")