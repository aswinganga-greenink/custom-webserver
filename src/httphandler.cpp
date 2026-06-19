#include "httphandler.hpp"
#include "httprequest.hpp"

#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "logger.hpp"

void HttpHandler::process_client(int client_fd, OnCompleteCallback on_complete) {
    char buffer[1024] = {0};


    int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if(bytes_read <= 0){
        on_complete(false); 
        return;
    }


    std::string raw_request(buffer);
    if (raw_request.empty()) {
        on_complete(false); 
        return;
    }

    HttpRequest req;
    if(!req.parse(raw_request)){
        LOG_WARN("Recieved malinformed HTTP request. Dropping connection.");
        on_complete(false);
        return;
    }

    LOG_INFO("Parsed " + req.method + " request for " + req.uri);

    bool keep_alive = false;
    if(req.headers.count("Connection") && req.headers["Connection"] == "keep-alive"){
        keep_alive = true;
        LOG_DEBUG("Browser requested Keep-Alive on FD " + std::to_string(client_fd));
    }

    std::string response = build_response(req.uri, keep_alive);
    write(client_fd, response.c_str(), response.size());

    on_complete(keep_alive);
}

std::string HttpHandler::build_response(const std::string& uri, bool keep_alive) {
    
    std::filesystem::path base_dir;

    try{
        base_dir = std::filesystem::canonical("public");
    }catch(const std::exception& e){
        LOG_ERROR("CRITICAL: 'public' directory is missing!");
        return "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n";
    }

    std::string safe_uri = uri;
    if (!safe_uri.empty() && safe_uri[0] == '/') {
        safe_uri = safe_uri.substr(1);
    }
    if (safe_uri.empty()) {
        safe_uri = "index.html"; 
    }

    std::filesystem::path target_path = base_dir / safe_uri;
    std::filesystem::path resolved_path = std::filesystem::weakly_canonical(target_path);

    std::string filepath = "public" + uri;

    if (resolved_path.string().find(base_dir.string()) != 0) {
        LOG_WARN("SECURITY ALERT: Directory traversal blocked -> " + uri);
        std::string error_body = "<html><body><h1>403 Forbidden</h1></body></html>";
        return "HTTP/1.1 403 Forbidden\r\n"
               "Content-Type: text/html\r\n"
               "Content-Length: " + std::to_string(error_body.length()) + "\r\n"
               "Connection: close\r\n\r\n" + error_body;
    }
    
     std::ifstream file(filepath, std::ios::binary);

    if (file.is_open()) {
        std::stringstream buffer_stream;
        buffer_stream << file.rdbuf();
        std::string content = buffer_stream.str();

        std::string mime_type = get_mime_type(filepath);
        
        std::string conn_header = keep_alive ? "keep-alive" : "close";

        return "HTTP/1.1 200 OK\r\n"
               "Content-Type: " + mime_type + "\r\n"
               "Content-Length: " + std::to_string(content.length()) + "\r\n"
               "Connection: " + conn_header + "\r\n\r\n" + 
               content;
    }

    LOG_ERROR("404 Not Found -> " + filepath);
    std::string error_body = "<html><body style='font-family:monospace;'><h1>404 File Not Found</h1></body></html>";
    
    return "HTTP/1.1 404 Not Found\r\n"
           "Content-Type: text/html\r\n"
           "Content-Length: " + std::to_string(error_body.length()) + "\r\n"
           "Connection: close\r\n\r\n" +
           error_body;
}

std::string HttpHandler::extract_path(const std::string& raw_request) {
    std::istringstream stream(raw_request);
    std::string        method, path, protocol;

    stream >> method >> path >> protocol;

    if (path == "/") {
        path = "/index.html";
    }

    return "public" + path;
}

std::string HttpHandler::get_mime_type(const std::string& filepath) {
    
    if (filepath.find(".html") != std::string::npos) return "text/html";
    if (filepath.find(".css") != std::string::npos) return "text/css";
    if (filepath.find(".js") != std::string::npos) return "application/javascript";
    if (filepath.find(".png") != std::string::npos) return "image/png";
    if (filepath.find(".jpg") != std::string::npos || filepath.find(".jpeg") != std::string::npos) return "image/jpeg";
    if (filepath.find(".ico") != std::string::npos) return "image/x-icon";
    
    return "application/octet-stream"; 
}