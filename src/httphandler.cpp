#include "httphandler.hpp"
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>


void HttpHandler::process_client(int client_fd){

    char buffer[1024] = {0};
    read(client_fd, buffer, sizeof(buffer) - 1 );
    std::string raw_request(buffer);

    std::string requested_path = "public/index.html";

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

        return "HTTP/1.1 200 OK\r\n"
               "Content-Type: text/html\r\n"
               "Content-Length: " + std::to_string(content.length()) + "\r\n"
               "Connection: close\r\n\r\n" + 
               content;
    }

    return "HTTP/1.1 404 Not Found\r\n"
           "Content-Type: text/plain\r\n"
           "Connection: close\r\n\r\n"
           "404 - File Not Found";

}