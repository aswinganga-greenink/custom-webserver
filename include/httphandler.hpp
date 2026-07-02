#pragma once
#include <functional>
#include <string>

#include "httphandler.hpp"
#include "session.hpp"
#include "config.hpp"

class HttpHandler {
   public:
    using OnCompleteCallback = std::function<void(bool keep_alive)>;
    void process_client(Session* session, OnCompleteCallback on_complete);
    void process_proxy(Session* session, int ready_fd, OnCompleteCallback on_complete);
    HttpHandler(const ConfigParser& config);

   private:
    ConfigParser config;
    std::string extract_path(const std::string& raw_request);

    std::string build_response(const std::string& uri, bool keep_alive);
    std::string get_mime_type(const std::string& filepath);
};