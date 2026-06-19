#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace AntiDetect {

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

class Logger {
public:
    static Logger& getInstance();
    
    void setLogLevel(LogLevel level);
    void setLogFile(const std::string& filepath);
    void setConsoleOutput(bool enable);
    
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);
    
    void log(LogLevel level, const std::string& message);
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger();
    ~Logger();
    
    std::string levelToString(LogLevel level);
    std::string getCurrentTimestamp();
    void writeToFile(const std::string& formattedMessage);
    
    LogLevel m_minLevel;
    std::string m_logFilePath;
    std::ofstream m_fileStream;
    std::mutex m_mutex;
    bool m_consoleOutput;
};

}
