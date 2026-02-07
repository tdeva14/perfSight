#pragma once

#include <iostream>
#include <fstream>
#include <mutex>
#include <sstream>
#include <ctime>
#include <iomanip>

namespace perfsight {
namespace core {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

/**
 * @brief Thread-safe singleton logger
 */
class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void init(const std::string& logPath, LogLevel level = LogLevel::INFO) {
        std::lock_guard<std::mutex> lock(mutex_);
        logLevel_ = level;
        logFilePath_ = logPath;
        
        // Create log file
        logFile_.open(logPath, std::ios::app);
        if (!logFile_.is_open()) {
            std::cerr << "Failed to open log file: " << logPath << std::endl;
        }
    }

    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        logLevel_ = level;
    }

    template<typename... Args>
    void debug(Args&&... args) {
        log(LogLevel::DEBUG, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(Args&&... args) {
        log(LogLevel::INFO, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warning(Args&&... args) {
        log(LogLevel::WARNING, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(Args&&... args) {
        log(LogLevel::ERROR, std::forward<Args>(args)...);
    }

private:
    Logger() : logLevel_(LogLevel::INFO) {}
    ~Logger() {
        if (logFile_.is_open()) {
            logFile_.close();
        }
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    template<typename... Args>
    void log(LogLevel level, Args&&... args) {
        if (level < logLevel_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        
        std::ostringstream oss;
        oss << getTimestamp() << " [" << levelToString(level) << "] ";
        appendArgs(oss, std::forward<Args>(args)...);
        
        std::string message = oss.str();
        
        // Write to console
        if (level >= LogLevel::ERROR) {
            std::cerr << message << std::endl;
        } else {
            std::cout << message << std::endl;
        }
        
        // Write to file
        if (logFile_.is_open()) {
            logFile_ << message << std::endl;
            logFile_.flush();
        }
    }

    template<typename T>
    void appendArgs(std::ostringstream& oss, T&& arg) {
        oss << std::forward<T>(arg);
    }

    template<typename T, typename... Args>
    void appendArgs(std::ostringstream& oss, T&& arg, Args&&... args) {
        oss << std::forward<T>(arg);
        appendArgs(oss, std::forward<Args>(args)...);
    }

    std::string getTimestamp() {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

    std::mutex mutex_;
    std::ofstream logFile_;
    std::string logFilePath_;
    LogLevel logLevel_;
};

} // namespace core
} // namespace perfsight
