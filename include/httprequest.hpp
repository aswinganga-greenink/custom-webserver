#pragma once
#include <string>
#include <unordered_map>


class HttpRequest {
    public:
        std::string method;
        std::string uri;
        std::string version;

        std::unordered_map<std::string, std::string> headers;

        bool parse(const std::string& raw_request);
};

