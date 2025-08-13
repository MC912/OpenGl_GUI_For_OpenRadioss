#include "gui/GuiManager.h"
#include "core/Application.h"
#include "core/Model.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

GuiManager::GuiManager(GLFWwindow* window, Application* app)
    : m_Window(window), m_Application(app) {
}

void GuiManager::Initialize() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void GuiManager::DrawMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                m_ShowFileDialog = true;
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                if (m_FileSaveCallback) {
                    m_FileSaveCallback(m_CurrentPath);
                }
            }
            if (ImGui::MenuItem("Save As...")) {
                m_ShowFileDialog = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Import...")) {
                // Import other formats
            }
            if (ImGui::MenuItem("Export...")) {
                // Export other formats
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                glfwSetWindowShouldClose(m_Window, true);
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Select All", "Ctrl+A")) {}
            if (ImGui::MenuItem("Delete", "Del")) {}
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Wireframe", "W")) {}
            if (ImGui::MenuItem("Solid", "S")) {}
            if (ImGui::MenuItem("Nodes", "N")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Property Panel", nullptr, m_ShowPropertyPanel)) {
                m_ShowPropertyPanel = !m_ShowPropertyPanel;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Solver")) {
            if (ImGui::MenuItem("Run OpenRadioss", "F5")) {
                m_ShowSolverDialog = true;
            }
            if (ImGui::MenuItem("Solver Settings...")) {}
            if (ImGui::MenuItem("Job Manager...")) {}
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Documentation")) {}
            if (ImGui::MenuItem("About")) {
                m_ShowAboutDialog = true;
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void GuiManager::DrawToolBar() {
    ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoTitleBar | 
                                     ImGuiWindowFlags_NoResize);
    
    if (ImGui::Button("Open")) {
        m_ShowFileDialog = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        if (m_FileSaveCallback && !m_CurrentPath.empty()) {
            m_FileSaveCallback(m_CurrentPath);
        }
    }
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    
    if (ImGui::Button("Run Solver")) {
        m_ShowSolverDialog = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        // Stop current job
    }
    
    ImGui::End();
}

void GuiManager::DrawSolverDialog() {
    if (!m_ShowSolverDialog) return;
    
    ImGui::Begin("Solver Settings", &m_ShowSolverDialog);
    
    static char solverPath[256] = "/usr/local/bin/openradioss";
    ImGui::InputText("Solver Path", solverPath, sizeof(solverPath));
    
    static int numCPUs = 4;
    ImGui::SliderInt("Number of CPUs", &numCPUs, 1, 16);
    
    static bool useMPI = false;
    ImGui::Checkbox("Use MPI", &useMPI);
    
    static float endTime = 1.0f;
    ImGui::InputFloat("End Time", &endTime);
    
    ImGui::Separator();
    
    if (ImGui::Button("Run")) {
        if (m_SolverRunCallback) {
            m_SolverRunCallback();
        }
        m_ShowSolverDialog = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        m_ShowSolverDialog = false;
    }
    
    ImGui::End();
}