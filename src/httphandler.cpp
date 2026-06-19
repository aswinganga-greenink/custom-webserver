#include "httphandler.hpp"
#include "httprequest.hpp"

#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "logger.hpp"

void HttpHandler::process_client(int client_fd) {
    char buffer[1024] = {0};


    int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if(bytes_read <= 0){
        close(client_fd);
        return;
    }


    std::string raw_request(buffer);
    if (raw_request.empty()) {
        close(client_fd);
        return;
    }

    HttpRequest req;
    if(!req.parse(raw_request)){
        LOG_WARN("Recieved malinformed HTTP request. Dropping connection.");
        close(client_fd);
        return;
    }

    LOG_INFO("Parsed " + req.method + " request for " + req.uri);

    bool keep_alive = false;
    if(req.headers.count("Connection") && req.headers["Connection"] == "keep-alive"){
        keep_alive = true;
        LOG_DEBUG("Browser requested Keep-Alive on FD " + std::to_string(client_fd));
    }

    std::string response = build_response(req.uri);
    write(client_fd, response.c_str(), response.size());

    if(!keep_alive){
        close(client_fd);
    }
    else{
        close(client_fd); // for now
    }
}

std::string HttpHandler::build_response(const std::string& filepath) {
    std::ifstream file(filepath);

    if (file.good()) {
        std::stringstream buffer_stream;
        buffer_stream << file.rdbuf();
        std::string content = buffer_stream.str();

        std::string mime_type = get_mime_type(filepath);

        return "HTTP/1.1 200 OK\r\n"
               "Content-Type: " +
               mime_type +
               "\r\n"
               "Content-Length: " +
               std::to_string(content.length()) +
               "\r\n"
               "Connection: close\r\n\r\n" +
               content;
    }

    LOG_ERROR("404 Not Found -> " + filepath);

    return "HTTP/1.1 404 Not Found\r\n"
           "Content-Type: text/plain\r\n"
           "Connection: close\r\n\r\n"
           "404 - File Not Found";
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
    size_t dot_pos = filepath.find_last_of(".");

    if (dot_pos == std::string::npos) {
        return "text/plain";
    }

    std::string ext = filepath.substr(dot_pos);

    if (ext == ".html") return "text/html";
    if (ext == ".css") return "text/css";
    if (ext == ".js") return "application/javascript";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";

    return "text/plain";
}