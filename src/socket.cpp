#include "socket.hpp"
#include "logger.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <errno.h>   
#include <string.h>  
#include <stdlib.h>


Socket::Socket(int port){
    this->port = port;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
}

Socket::Socket(Socket&& other){
    this->port = other.port;
    this->sock_fd = other.sock_fd;
    other.sock_fd = -1;
    other.port = 0;
}

Socket::~Socket(){
    if( sock_fd != -1){
        close(sock_fd);
    }

}

void Socket::set_content(){
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    server.sin_family = AF_INET;
}

void Socket::bind_sock(){
    set_content();
    
    if (bind(sock_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        LOG_ERROR("CRITICAL ERROR: Failed to bind to port " + port + '!');
        LOG_ERROR("OS Reason: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE); 
    }
    LOG_INFO("Successfully bound to port " + port);
}

void Socket::listen_sock(){
    if (listen(sock_fd, 5) < 0) {
        LOG_ERROR("CRITICAL ERROR: Failed to listen on socket!");
        LOG_ERROR("OS Reason: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }
    LOG_INFO("Socket is actively listening...");
}

int Socket::accept_sock(){
    socklen_t client_size = sizeof(client);
    int client_fd = accept(sock_fd, (struct sockaddr*)&client, &client_size);
    return client_fd;
}

