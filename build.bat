@echo off
setlocal enabledelayedexpansion

:: Configuration
set BUILD_TYPE=Release
set BUILD_DIR=build
set GENERATOR="Visual Studio 16 2019"

:: Parse arguments
:parse_args
if "%1"=="" goto :check_deps
if "%1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto :parse_args
)
if "%1"=="--clean" (
    echo Cleaning build directory...
    if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%
    shift
    goto :parse_args
)
if "%1"=="--help" (
    echo Usage: build.bat [options]
    echo Options:
    echo   --debug          Build in debug mode
    echo   --clean          Clean build directory
    echo   --help           Show this help message
    exit /b 0
)
echo Unknown option: %1
exit /b 1

:check_deps
echo Checking dependencies...

:: Check for CMake
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo CMake is not installed. Please install CMake 3.16 or higher.
    exit /b 1
)

:: Download dependencies if not present
if not exist "dependencies\glfw" (
    echo Downloading GLFW...
    git clone https://github.com/glfw/glfw.git dependencies\glfw
)

if not exist "dependencies\glm" (
    echo Downloading GLM...
    git clone https://github.com/g-truc/glm.git dependencies\glm
)

if not exist "dependencies\imgui" (
    echo Downloading Dear ImGui...
    git clone https://github.com/ocornut/imgui.git dependencies\imgui
)

if not exist "dependencies\json" (
    echo Downloading JSON library...
    git clone https://github.com/nlohmann/json.git dependencies\json
)

:: Create build directory
echo Creating build directory...
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

:: Configure with CMake
echo Configuring with CMake...
cmake .. -G %GENERATOR% -A x64 ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DBUILD_TESTS=ON ^
    -DBUILD_EXAMPLES=ON

if %errorlevel% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

:: Build
echo Building...
cmake --build . --config %BUILD_TYPE%

if %errorlevel% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build completed successfully!
echo Executable: %BUILD_DIR%\%BUILD_TYPE%\OpenRadiossPreProcessor.exe

cd ..
echo Done!
