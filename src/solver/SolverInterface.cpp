#include "solver/SolverInterface.h"
#include "utils/Logger.h"
#include <sstream>
#include <cstdlib>
#include <iostream>

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
#else
    #include <unistd.h>
    #include <sys/wait.h>
#endif

SolverInterface::SolverInterface() 
    : m_Status(SolverStatus::IDLE), m_Progress(0.0f), m_CancelRequested(false) {
}

std::string SolverInterface::BuildCommandLine() const {
    std::stringstream cmd;
    
    // Build starter command
    cmd << m_Config.solverPath << "/starter_linux64_gf ";
    cmd << "-i " << m_Config.inputFile << " ";
    cmd << "-np " << m_Config.numProcessors;
    
    return cmd.str();
}

void SolverInterface::RunSolverAsync() {
    if (m_Status == SolverStatus::RUNNING) {
        LOG_WARN("Solver is already running");
        return;
    }
    
    m_Status = SolverStatus::RUNNING;
    m_Progress = 0.0f;
    m_CancelRequested = false;
    
    m_SolverThread = std::thread(&SolverInterface::ExecuteSolver, this);
}

void SolverInterface::ExecuteSolver() {
    LOG_INFO("Starting OpenRadioss solver...");
    
    std::string starterCmd = BuildCommandLine();
    LOG_INFO("Executing: {}", starterCmd);
    
    // Execute starter
    FILE* pipe = popen(starterCmd.c_str(), "r");
    if (!pipe) {
        LOG_ERROR("Failed to execute solver");
        m_Status = SolverStatus::ERROR;
        if (m_CompletionCallback) {
            m_CompletionCallback(false);
        }
        return;
    }
    
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        if (m_CancelRequested) {
            pclose(pipe);
            m_Status = SolverStatus::CANCELLED;
            LOG_INFO("Solver cancelled by user");
            break;
        }
        
        std::string line(buffer);
        m_OutputLog += line;
        
        ParseOutput(line);
        
        if (m_LogCallback) {
            m_LogCallback(line);
        }
    }
    
    int result = pclose(pipe);
    
    if (!m_CancelRequested) {
        if (result == 0) {
            // Now run engine
            std::stringstream engineCmd;
            if (m_Config.useMPI) {
                engineCmd << "mpirun -np " << m_Config.numProcessors << " ";
                engineCmd << m_Config.solverPath << "/engine_linux64_gf_ompi ";
            } else {
                engineCmd << m_Config.solverPath << "/engine_linux64_gf ";
            }
            engineCmd << "-i " << m_Config.inputFile << "_0001.rad";
            
            LOG_INFO("Executing engine: {}", engineCmd.str());
            
            pipe = popen(engineCmd.str().c_str(), "r");
            if (pipe) {
                while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                    if (m_CancelRequested) {
                        pclose(pipe);
                        m_Status = SolverStatus::CANCELLED;
                        break;
                    }
                    
                    std::string line(buffer);
                    m_OutputLog += line;
                    ParseOutput(line);
                    
                    if (m_LogCallback) {
                        m_LogCallback(line);
                    }
                }
                
                result = pclose(pipe);
                if (result == 0 && !m_CancelRequested) {
                    m_Status = SolverStatus::COMPLETED;
                    m_Progress = 1.0f;
                    LOG_INFO("Solver completed successfully");
                    
                    if (m_CompletionCallback) {
                        m_CompletionCallback(true);
                    }
                }
            }
        } else {
            m_Status = SolverStatus::ERROR;
            LOG_ERROR("Solver failed with code: {}", result);
            
            if (m_CompletionCallback) {
                m_CompletionCallback(false);
            }
        }
    }
}

void SolverInterface::ParseOutput(const std::string& line) {
    // Parse OpenRadioss output for progress
    if (line.find("CYCLE") != std::string::npos) {
        // Extract cycle number and calculate progress
        std::size_t pos = line.find("CYCLE");
        if (pos != std::string::npos) {
            std::istringstream iss(line.substr(pos + 5));
            int cycle;
            if (iss >> cycle) {
                // Estimate progress (this is simplified)
                m_Progress = std::min(1.0f, cycle / 10000.0f);
                
                if (m_ProgressCallback) {
                    m_ProgressCallback(m_Progress);
                }
            }
        }
    }
}