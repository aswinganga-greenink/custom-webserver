#pragma once
#include <iostream>
#include <string>

#include "logger.hpp"

class ConfigParser {
   public:
    int         port           = 8000;
    int         worker_threads = 4;
    std::string document_root  = "/public";
    LogLevel    log_level      = LogLevel::INFO;

    void load_from_stream(std::istream& stream);
    bool load_from_file(const std::string& filename);
};