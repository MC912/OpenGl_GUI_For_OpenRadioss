# CMakeLists.txt - Fixed version
cmake_minimum_required(VERSION 3.16)
project(OpenRadiossPreProcessor VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Build tests" OFF)  # Set to OFF for now
option(BUILD_EXAMPLES "Build example models" ON)

# Find packages
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# Find GLEW - Use system package or download
find_package(GLEW)
if(NOT GLEW_FOUND)
    message(STATUS "GLEW not found in system, using bundled version")
    # For bundled GLEW, we'll compile it directly
    set(GLEW_SOURCES
        dependencies/glew/src/glew.c
    )
    set(GLEW_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/dependencies/glew/include)
    set(GLEW_LIBRARIES "")  # We'll compile it with our project
else()
    message(STATUS "Using system GLEW")
    set(GLEW_SOURCES "")
endif()

# Check and add GLFW
if(EXISTS ${PROJECT_SOURCE_DIR}/dependencies/glfw/CMakeLists.txt)
    add_subdirectory(dependencies/glfw)
else()
    find_package(glfw3 3.3 REQUIRED)
endif()

# ImGui sources
set(IMGUI_SOURCES
    dependencies/imgui/imgui.cpp
    dependencies/imgui/imgui_demo.cpp
    dependencies/imgui/imgui_draw.cpp
    dependencies/imgui/imgui_widgets.cpp
    dependencies/imgui/imgui_tables.cpp
    dependencies/imgui/backends/imgui_impl_glfw.cpp
    dependencies/imgui/backends/imgui_impl_opengl3.cpp
)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/dependencies/glfw/include
    ${PROJECT_SOURCE_DIR}/dependencies/glm
    ${PROJECT_SOURCE_DIR}/dependencies/imgui
    ${PROJECT_SOURCE_DIR}/dependencies/imgui/backends
    ${PROJECT_SOURCE_DIR}/dependencies/json/single_include
)

# Collect source files
file(GLOB_RECURSE PROJECT_SOURCES 
    "src/*.cpp"
    "src/*.h"
)

# Create executable
add_executable(${PROJECT_NAME} 
    ${PROJECT_SOURCES}
    ${IMGUI_SOURCES}
    ${GLEW_SOURCES}
)

# Link libraries
if(GLEW_FOUND)
    target_link_libraries(${PROJECT_NAME}
        ${OPENGL_LIBRARIES}
        ${GLEW_LIBRARIES}
        glfw
        ${CMAKE_THREAD_LIBS_INIT}
        ${CMAKE_DL_LIBS}
    )
else()
    target_link_libraries(${PROJECT_NAME}
        ${OPENGL_LIBRARIES}
        glfw
        ${CMAKE_THREAD_LIBS_INIT}
        ${CMAKE_DL_LIBS}
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)
endif()

# Platform specific settings
if(WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    target_link_libraries(${PROJECT_NAME} opengl32 gdi32 shell32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} GL X11 pthread dl)
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    target_link_libraries(${PROJECT_NAME} ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY})
endif()

# Copy resources to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})

# Tests (only if directory exists)
if(BUILD_TESTS AND EXISTS ${PROJECT_SOURCE_DIR}/tests/CMakeLists.txt)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation rules
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY shaders DESTINATION share/${PROJECT_NAME})
install(DIRECTORY resources DESTINATION share/${PROJECT_NAME})
