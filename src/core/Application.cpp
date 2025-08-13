// src/core/Application.cpp
#include "core/Application.h"
#include "core/Model.h"
#include "rendering/Renderer.h"
#include "gui/GuiManager.h"
#include "io/FileManager.h"
#include "solver/SolverInterface.h"
#include "utils/Logger.h"
#include <GLFW/glfw3.h>
#include <chrono>

Application::Application() 
    : m_Running(false), m_LastFrameTime(0.0f) {
    Initialize();
}

Application::~Application() {
    Shutdown();
}

void Application::Initialize() {
    LOG_INFO("Initializing application...");
    
    // Initialize GLFW
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    
    // Create window with OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(1600, 900, 
        "OpenRadioss Pre-Processor", nullptr, nullptr);
    
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create window");
    }
    
    glfwMakeContextCurrent(window);
    
    // Initialize components
    m_Model = std::make_unique<Model>();
    m_Renderer = std::make_unique<Renderer>(window);
    m_GuiManager = std::make_unique<GuiManager>(window, this);
    m_FileManager = std::make_unique<FileManager>(m_Model.get());
    m_SolverInterface = std::make_unique<SolverInterface>();
    
    // Setup callbacks
    m_GuiManager->SetFileOpenCallback(
        [this](const std::string& path) { LoadFile(path); });
    
    m_GuiManager->SetFileSaveCallback(
        [this](const std::string& path) { SaveFile(path); });
    
    m_GuiManager->SetSolverRunCallback(
        [this]() { 
            SolverConfig config;
            config.inputFile = m_FileManager->GetCurrentFile();
            config.numProcessors = 4;
            m_SolverInterface->SetConfig(config);
            m_SolverInterface->RunSolverAsync();
        });
    
    // Initialize GUI
    m_GuiManager->Initialize();
    
    LOG_INFO("Application initialized successfully");
}

void Application::Run() {
    m_Running = true;
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (m_Running && !glfwWindowShouldClose(m_Renderer->GetWindow())) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        ProcessInput();
        Update(deltaTime);
        Render();
        
        glfwSwapBuffers(m_Renderer->GetWindow());
        glfwPollEvents();
    }
}

void Application::ProcessInput() {
    GLFWwindow* window = m_Renderer->GetWindow();
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        m_Running = false;
    }
}

void Application::Update(float deltaTime) {
    m_Renderer->Update(deltaTime);
    
    // Update solver status if running
    if (m_SolverInterface->GetStatus() == SolverStatus::RUNNING) {
        // Status updates handled by callbacks
    }
}

void Application::Render() {
    m_Renderer->BeginFrame();
    
    // Render 3D scene
    m_Renderer->RenderModel(m_Model.get());
    
    // Render GUI
    m_GuiManager->BeginFrame();
    m_GuiManager->DrawMenuBar();
    m_GuiManager->DrawToolBar();
    m_GuiManager->DrawPropertyPanel();
    m_GuiManager->DrawStatusBar();
    m_GuiManager->DrawSolverDialog();
    m_GuiManager->EndFrame();
    
    m_Renderer->EndFrame();
}

void Application::LoadFile(const std::string& filepath) {
    LOG_INFO("Loading file: {}", filepath);
    
    try {
        m_FileManager->LoadRadFile(filepath);
        m_Renderer->UpdateMesh(m_Model.get());
        LOG_INFO("File loaded successfully");
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load file: {}", e.what());
    }
}

void Application::SaveFile(const std::string& filepath) {
    LOG_INFO("Saving file: {}", filepath);
    
    try {
        m_FileManager->SaveRadFile(filepath);
        LOG_INFO("File saved successfully");
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to save file: {}", e.what());
    }
}

void Application::Shutdown() {
    LOG_INFO("Shutting down application...");
    
    m_GuiManager->Shutdown();
    m_Renderer->Shutdown();
    
    glfwTerminate();
}