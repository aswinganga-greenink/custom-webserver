#include <iostream>
#include "server.hpp"
#include "socket.hpp"
#include <unistd.h>

Server::Server(int port): port(port), sock(port), pool(4) {}

Server::~Server(){
    std::cout << "Server shutting down..." << std::endl;
}

void Server::start_server(){
    std::cout<<"Starting server on port " << port << std::endl;

    sock.bind_sock();

    sock.listen_sock();

    std::cout << "Server is now running and waiting for connections." << std::endl;

    while(true){
        int current_client_fd = sock.accept_sock();

        pool.enqueue_task([current_client_fd](){
            std::cout<< "Worker thread processing client FD: " << current_client_fd << std::endl;


            char buffer[1024] = {0};
            read(current_client_fd, buffer, sizeof(buffer) - 1 );

            std::cout<< buffer << std::endl;

            std::string http_response = 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n"
                "\r\n"
                "<html>"
                "<head><title>My C++ Server</title></head>"
                "<body style='background-color: #222; color: #00FF00; font-family: monospace;'>"
                "<h1>Connection Successful!</h1>"
                "<p>This page was served dynamically by a custom C++ ThreadPool.</p>"
                "</body>"
                "</html>";

            write(current_client_fd, http_response.c_str(), http_response.length());

            close(current_client_fd);
            std::cout<<"Connection Closed\n" << std::endl;

        });

        std::cout << "Client connected!" << std::endl;
    }
}