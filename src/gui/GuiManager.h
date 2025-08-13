#pragma once
#include <memory>
#include <functional>
#include <string>

struct GLFWwindow;
class Application;

class GuiManager {
public:
    GuiManager(GLFWwindow* window, Application* app);
    ~GuiManager();
    
    void Initialize();
    void BeginFrame();
    void EndFrame();
    void Render();
    void Shutdown();
    
    // Main GUI components
    void DrawMenuBar();
    void DrawToolBar();
    void DrawPropertyPanel();
    void DrawStatusBar();
    void DrawSolverDialog();
    
    // Callbacks
    void SetFileOpenCallback(std::function<void(const std::string&)> callback);
    void SetFileSaveCallback(std::function<void(const std::string&)> callback);
    void SetSolverRunCallback(std::function<void()> callback);
    
private:
    void DrawFileDialog();
    void DrawAboutDialog();
    
private:
    GLFWwindow* m_Window;
    Application* m_Application;
    
    // Dialog states
    bool m_ShowFileDialog = false;
    bool m_ShowAboutDialog = false;
    bool m_ShowSolverDialog = false;
    bool m_ShowPropertyPanel = true;
    
    // Callbacks
    std::function<void(const std::string&)> m_FileOpenCallback;
    std::function<void(const std::string&)> m_FileSaveCallback;
    std::function<void()> m_SolverRunCallback;
    
    // File dialog
    std::string m_CurrentPath;
    std::string m_SelectedFile;
};