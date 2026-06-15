#include "logger.hpp"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

std::mutex Logger::log_mutex;

std::string Logger::get_timestamp(){
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    char buffer[26];
    struct tm* timeinfo = std::localtime(&now_c);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    return std::string(buffer);
}

std::string Logger::level_to_color(LogLevel level){
    switch (level) {
        case LogLevel::INFO:    return "\033[32m";
        case LogLevel::WARNING: return "\033[33m"; 
        case LogLevel::ERROR:   return "\033[31m";
        case LogLevel::DEBUG:   return "\033[36m"; 
        default:                return "\033[0m";
    }
}

std::string Logger::level_to_string(LogLevel level){
    switch (level) {
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::DEBUG:   return "DEBUG";
        default:                return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex);

    std::string color = level_to_color(level);
    std::string reset = "\033[0m";

    std::cout << color << "[" << get_timestamp() << "] "
              << "[" << level_to_string(level) << "] "
              << reset << message << std::endl;
}