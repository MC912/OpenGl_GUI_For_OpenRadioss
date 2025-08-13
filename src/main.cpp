#include "core/Application.h"
#include "utils/Logger.h"
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    // Initialize logger
    Logger::Init();
    
    LOG_INFO("OpenRadioss Pre-Processor starting...");
    
    // Create and run application
    try {
        auto app = std::make_unique<Application>();
        
        // Parse command line arguments
        if (argc > 1) {
            app->LoadFile(argv[1]);
        }
        
        // Run main loop
        app->Run();
        
    } catch (const std::exception& e) {
        LOG_ERROR("Application error: {}", e.what());
        return -1;
    }
    
    LOG_INFO("Application terminated successfully");
    return 0;
}