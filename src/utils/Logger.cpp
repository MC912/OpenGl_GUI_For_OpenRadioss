#include "utils/Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>

std::unique_ptr<Logger> Logger::s_Instance = nullptr;

void Logger::Init() {
    if (!s_Instance) {
        s_Instance = std::unique_ptr<Logger>(new Logger());
    }
}

void Logger::Shutdown() {
    s_Instance.reset();
}

Logger* Logger::GetInstance() {
    if (!s_Instance) {
        Init();
    }
    return s_Instance.get();
}

Logger::Logger() : m_Level(LogLevel::INFO) {
}

Logger::~Logger() {
}

void Logger::LogMessage(LogLevel level, const std::string& message) {
    if (level < m_Level) return;
    
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    // Level strings
    const char* levelStr[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"
    };
    
    // Color codes for terminal
    const char* colorCodes[] = {
        "\033[37m",  // White for TRACE
        "\033[36m",  // Cyan for DEBUG
        "\033[32m",  // Green for INFO
        "\033[33m",  // Yellow for WARN
        "\033[31m",  // Red for ERROR
        "\033[35m"   // Magenta for CRITICAL
    };
    
    // Print log message
    std::cout << colorCodes[static_cast<int>(level)]
              << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S") << "] "
              << "[" << levelStr[static_cast<int>(level)] << "] "
              << message
              << "\033[0m"  // Reset color
              << std::endl;
}