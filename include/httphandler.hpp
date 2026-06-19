#pragma once
#include <string>
#include "httphandler.hpp"
#include <functional>

class HttpHandler {
   public:
    using OnCompleteCallback = std::function<void()>;
    void process_client(int client_fd, OnCompleteCallback on_complete);
    

   private:
    std::string extract_path(const std::string& raw_request);

    std::string build_response(const std::string& filepath);
    std::string get_mime_type(const std::string& filepath);
};