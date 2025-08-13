cmake_minimum_required(VERSION 3.16)
project(OpenRadiossPreProcessor VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build example models" ON)

# Find packages
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# Add subdirectories
add_subdirectory(dependencies/glfw)
add_subdirectory(dependencies/glew)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/dependencies/glfw/include
    ${PROJECT_SOURCE_DIR}/dependencies/glew/include
    ${PROJECT_SOURCE_DIR}/dependencies/glm
    ${PROJECT_SOURCE_DIR}/dependencies/imgui
    ${PROJECT_SOURCE_DIR}/dependencies/json/include
)

# Source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "dependencies/imgui/*.cpp"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    glfw
    ${GLEW_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

# Copy resources to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})

# Platform specific settings
if(WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()