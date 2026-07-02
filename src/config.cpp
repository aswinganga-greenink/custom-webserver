#include "config.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>

#include "logger.hpp"

static void trim(std::string& s) {
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));

    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            s.end());
}

void ConfigParser::load_from_stream(std::istream& stream) {
    std::string line;

    while (std::getline(stream, line)) {
        auto comment_pos = line.find("#");
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }

        trim(line);
        if (line.empty()) continue;

        auto delimiter_pos = line.find("=");
        if (delimiter_pos != std::string::npos) {
            std::string key   = line.substr(0, delimiter_pos);
            std::string value = line.substr(delimiter_pos + 1);

            trim(key);
            trim(value);

            if (key == "port") {
                port = std::stoi(value);
            } else if (key == "worker_threads") {
                worker_threads = std::stoi(value);
            } else if (key == "document_root") {
                document_root = value;
            } else if (key == "log_level") {
                std::transform(value.begin(), value.end(), value.begin(), ::toupper);

                if (value == "DEBUG")
                    log_level = LogLevel::DEBUG;
                else if (value == "INFO")
                    log_level = LogLevel::INFO;
                else if (value == "WARN")
                    log_level = LogLevel::WARN;
                else if (value == "ERROR")
                    log_level = LogLevel::ERROR;
                else if (value == "NONE")
                    log_level = LogLevel::NONE;

            } else if (key == "proxy_route") {
                proxy_route = value;
            } else if (key == "proxy_target_ip") {
                proxy_target_ip = value;
            } else if (key == "proxy_target_port") {
                proxy_target_port = std::stoi(value);
            }
        }
    }
}

bool ConfigParser::load_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        LOG_WARN("Config file '" + filename + "' not found. Using default architecture.");
        return false;
    }

    load_from_stream(file);
    LOG_INFO("Configuration loaded successfully from " + filename);
    return true;
}