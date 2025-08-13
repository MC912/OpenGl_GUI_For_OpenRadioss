#pragma once
#include <string>
#include <functional>
#include <thread>
#include <atomic>

struct SolverConfig {
    std::string solverPath;
    std::string workingDirectory;
    std::string inputFile;
    int numProcessors = 1;
    bool useMPI = false;
    float endTime = 1.0f;
    float timeStep = 0.0f;  // 0 = auto
};

enum class SolverStatus {
    IDLE,
    RUNNING,
    COMPLETED,
    ERROR,
    CANCELLED
};

class SolverInterface {
public:
    SolverInterface();
    ~SolverInterface();
    
    // Set configuration
    void SetConfig(const SolverConfig& config);
    
    // Run solver
    void RunSolver();
    void RunSolverAsync();
    void CancelSolver();
    
    // Status
    SolverStatus GetStatus() const { return m_Status; }
    float GetProgress() const { return m_Progress; }
    std::string GetOutputLog() const { return m_OutputLog; }
    
    // Callbacks
    void SetProgressCallback(std::function<void(float)> callback);
    void SetCompletionCallback(std::function<void(bool)> callback);
    void SetLogCallback(std::function<void(const std::string&)> callback);
    
private:
    void ExecuteSolver();
    void ParseOutput(const std::string& line);
    std::string BuildCommandLine() const;
    
private:
    SolverConfig m_Config;
    std::atomic<SolverStatus> m_Status;
    std::atomic<float> m_Progress;
    std::string m_OutputLog;
    
    std::thread m_SolverThread;
    std::atomic<bool> m_CancelRequested;
    
    // Callbacks
    std::function<void(float)> m_ProgressCallback;
    std::function<void(bool)> m_CompletionCallback;
    std::function<void(const std::string&)> m_LogCallback;
};