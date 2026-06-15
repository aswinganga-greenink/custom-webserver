#pragma once
#include <string>

class HttpHandler {
   public:
    void process_client(int client_fd);

   private:
    std::string extract_path(const std::string& raw_request);

    std::string build_response(const std::string& filepath);
    std::string get_mime_type(const std::string& filepath);
};