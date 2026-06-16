#pragma once
#include <mutex>
#include <string>

enum class LogLevel {     
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    NONE = 4 
};

class Logger {
   private:
    static std::mutex log_mutex;
    static LogLevel current_threshold;

    static std::string get_timestamp();
    static std::string level_to_string(LogLevel level);
    static std::string level_to_color(LogLevel level);

   public:
    static void set_level(LogLevel level);
    static void log(LogLevel level, const std::string& message);
};

#define LOG_DEBUG(msg) Logger::log(LogLevel::DEBUG, msg)
#define LOG_INFO(msg) Logger::log(LogLevel::INFO, msg)
#define LOG_WARN(msg) Logger::log(LogLevel::WARN, msg)
#define LOG_ERROR(msg) Logger::log(LogLevel::ERROR, msg)