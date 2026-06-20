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


    static std::string get_timestamp();
    static std::string level_to_color(LogLevel level);

   public:
    static LogLevel current_threshold;
    static void set_level(LogLevel level);
    static void log(LogLevel level, const std::string& message);
    static std::string level_to_string(LogLevel level);
};

#define LOG_DEBUG(msg) \
    if (LogLevel::DEBUG >= Logger::current_threshold) { \
        Logger::log(LogLevel::DEBUG, msg); \
    }

#define LOG_INFO(msg) \
    if (LogLevel::INFO >= Logger::current_threshold) { \
        Logger::log(LogLevel::INFO, msg); \
    }

#define LOG_WARN(msg) \
    if (LogLevel::WARN >= Logger::current_threshold) { \
        Logger::log(LogLevel::WARN, msg); \
    }

#define LOG_ERROR(msg) \
    if (LogLevel::ERROR >= Logger::current_threshold) { \
        Logger::log(LogLevel::ERROR, msg); \
    }

