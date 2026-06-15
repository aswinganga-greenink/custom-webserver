#include <iostream>
#include "server.hpp"
#include "socket.hpp"
#include "httphandler.hpp"
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


            HttpHandler handler;
            handler.process_client(current_client_fd);

        });

        std::cout << "Client connected!" << std::endl;
    }
}