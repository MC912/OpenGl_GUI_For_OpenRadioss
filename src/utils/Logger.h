#pragma once
#include <string>
#include <memory>
#include <sstream>

enum class LogLevel {
    TRACE = 0,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL
};

class Logger {
public:
    static void Init();
    static void Shutdown();
    
    template<typename... Args>
    static void Log(LogLevel level, const std::string& format, Args... args) {
        GetInstance()->LogMessage(level, FormatString(format, args...));
    }
    
private:
    Logger();
    ~Logger();
    
    static Logger* GetInstance();
    void LogMessage(LogLevel level, const std::string& message);
    
    template<typename... Args>
    static std::string FormatString(const std::string& format, Args... args) {
        // Simplified formatting - in production use fmt library
        std::stringstream ss;
        ss << format;
        return ss.str();
    }
    
private:
    static std::unique_ptr<Logger> s_Instance;
    LogLevel m_Level;
};

// Macros for easy logging
#define LOG_TRACE(...)    Logger::Log(LogLevel::TRACE, __VA_ARGS__)
#define LOG_DEBUG(...)    Logger::Log(LogLevel::DEBUG, __VA_ARGS__)
#define LOG_INFO(...)     Logger::Log(LogLevel::INFO, __VA_ARGS__)
#define LOG_WARN(...)     Logger::Log(LogLevel::WARN, __VA_ARGS__)
#define LOG_ERROR(...)    Logger::Log(LogLevel::ERROR, __VA_ARGS__)
#define LOG_CRITICAL(...) Logger::Log(LogLevel::CRITICAL, __VA_ARGS__)