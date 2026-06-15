#pragma once
#include <mutex>
#include <string>

enum class LogLevel { INFO, WARNING, ERROR, DEBUG };

class Logger {
   private:
    static std::mutex log_mutex;

    static std::string get_timestamp();
    static std::string level_to_string(LogLevel level);
    static std::string level_to_color(LogLevel level);

   public:
    static void log(LogLevel level, const std::string& message);
};

#define LOG_INFO(msg) Logger::log(LogLevel::INFO, msg)
#define LOG_WARN(msg) Logger::log(LogLevel::WARNING, msg)
#define LOG_ERROR(msg) Logger::log(LogLevel::ERROR, msg)