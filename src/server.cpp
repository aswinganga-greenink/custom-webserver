#include "server.hpp"

#include <errno.h>
#include <unistd.h>

#include <atomic>
#include <iostream>

#include "httphandler.hpp"
#include "logger.hpp"
#include "socket.hpp"

Server::Server(int port) : port(port), sock(port), pool(4) {}

Server::~Server() { LOG_INFO("Have a nice day"); }

void Server::start_server(std::atomic<bool>& is_running) {
    LOG_INFO("Starting server on port 8000");

    sock.bind_sock();

    sock.listen_sock();

    LOG_INFO("Server is now running and waiting for connections.");

    sock.set_non_blocking();

    while (is_running.load()) {
        int current_client_fd = sock.accept_sock();

        if (current_client_fd < 0) {
            if (errno == EINTR && !is_running.load()) {
                break;
            }
            continue;
        }

        pool.enqueue_task([current_client_fd]() {
            LOG_INFO("Worker thread processing client FD: " + std::to_string(current_client_fd));

            HttpHandler handler;
            handler.process_client(current_client_fd);
        });

        LOG_INFO("Client connected!");
    }
}