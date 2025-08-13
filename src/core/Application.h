#pragma once
#include <memory>
#include <string>

class Model;
class Renderer;
class GuiManager;
class FileManager;
class SolverInterface;

class Application {
public:
    Application();
    ~Application();
    
    void Run();
    void LoadFile(const std::string& filepath);
    void SaveFile(const std::string& filepath);
    void Shutdown();
    
    // Getters
    Model* GetModel() { return m_Model.get(); }
    Renderer* GetRenderer() { return m_Renderer.get(); }
    
private:
    void Initialize();
    void Update(float deltaTime);
    void Render();
    void ProcessInput();
    
private:
    std::unique_ptr<Model> m_Model;
    std::unique_ptr<Renderer> m_Renderer;
    std::unique_ptr<GuiManager> m_GuiManager;
    std::unique_ptr<FileManager> m_FileManager;
    std::unique_ptr<SolverInterface> m_SolverInterface;
    
    bool m_Running;
    float m_LastFrameTime;
};