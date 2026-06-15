#include "httphandler.hpp"
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>


void HttpHandler::process_client(int client_fd){

    char buffer[1024] = {0};
    read(client_fd, buffer, sizeof(buffer) - 1 );
    std::string raw_request(buffer);

    if(raw_request.empty()) {
        close(client_fd);
        return;
    }

    std::string requested_path = extract_path(raw_request);

    std::cout<<"the client requested file: " << requested_path << std::endl;

    std::string response = build_response(requested_path);

    write(client_fd, response.c_str(), response.length());
    close(client_fd);
}

std::string HttpHandler::build_response(const std::string& filepath){

    std::ifstream file(filepath);

    if (file.good()){
        std::stringstream buffer_stream;
        buffer_stream << file.rdbuf();
        std::string content = buffer_stream.str();

        std::string mime_type = get_mime_type(filepath);

        return "HTTP/1.1 200 OK\r\n"
               "Content-Type: " + mime_type + "\r\n"
               "Content-Length: " + std::to_string(content.length()) + "\r\n"
               "Connection: close\r\n\r\n" + 
               content;
    }

    return "HTTP/1.1 404 Not Found\r\n"
           "Content-Type: text/plain\r\n"
           "Connection: close\r\n\r\n"
           "404 - File Not Found";

}

std::string HttpHandler::extract_path(const std::string& raw_request){
    std::istringstream stream(raw_request);
    std::string method, path, protocol;

    stream >> method >> path >> protocol;

    if(path == "/"){
        path = "/index.html";
    }

    return "public" + path;
}

std::string HttpHandler::get_mime_type(const std::string& filepath) {
    size_t dot_pos = filepath.find_last_of(".");

    if(dot_pos == std::string::npos){
        return "text/plain";
    }

    std::string ext = filepath.substr(dot_pos);
    
    if(ext == ".html") return "text/html";
    if(ext == ".css") return "text/css";
    if(ext == ".js") return "application/javascript";
    if(ext == ".png") return "image/png";
    if(ext == ".jpg" || ext == ".jpeg") return "image/jpeg";

    return "text/plain";
}