#pragma once
#include <string>
#include <iostream>

class ConfigParser {
    public:
        int port = 8000;
        int worker_threads = 4;
        std::string document_root = "/public";

        void load_from_stream(std::istream& stream);
        bool load_from_file(const std::string& filename);
};