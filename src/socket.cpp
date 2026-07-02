#include "socket.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "logger.hpp"

Socket::Socket() {
    this->port = 0;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd == -1) {
        LOG_ERROR("Failed to create client socket: " + std::string(strerror(errno)));
    }
}

Socket::Socket(int port) {
    this->port = port;
    sock_fd    = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd == -1) {
        LOG_ERROR("Failed to create socket: " + std::string(strerror(errno)));
        return;
    }

    int opt = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        LOG_ERROR("setsockopt SO_REUSEADDR failed: " + std::string(strerror(errno)));
    } else {
        LOG_DEBUG("SO_REUSEADDR successfully applied to FD " + std::to_string(sock_fd));
    }
}

Socket::Socket(Socket&& other) {
    this->port    = other.port;
    this->sock_fd = other.sock_fd;
    other.sock_fd = -1;
    other.port    = 0;
}

int Socket::release_fd() {
    int fd = sock_fd;
    sock_fd = -1;
    return fd;
}

Socket::~Socket() {
    if (sock_fd != -1) {
        close(sock_fd);
    }
}

void Socket::set_content() {
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port        = htons(port);
    server.sin_family      = AF_INET;
}

void Socket::bind_sock() {
    set_content();

    if (bind(sock_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        LOG_ERROR("CRITICAL ERROR: Failed to bind to port " + std::to_string(port) + "!");
        LOG_ERROR("OS Reason: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }
    LOG_INFO("Successfully bound to port " + std::to_string(port));
}

void Socket::listen_sock() {
    if (listen(sock_fd, 5) < 0) {
        LOG_ERROR("CRITICAL ERROR: Failed to listen on socket!");
        LOG_ERROR("OS Reason: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }
    LOG_INFO("Socket is actively listening...");
}

int Socket::accept_sock() {
    socklen_t client_size = sizeof(client);
    int       client_fd   = accept(sock_fd, (struct sockaddr*)&client, &client_size);
    return client_fd;
}

void Socket::set_non_blocking() {
    int flags = fcntl(sock_fd, F_GETFL, 0);
    if (flags == -1) {
        LOG_ERROR("Failed to get socket flags");
        return;
    }

    flags |= O_NONBLOCK;

    if (fcntl(sock_fd, F_SETFL, flags) == -1) {
        LOG_ERROR("failed to set socket to non blocking");
    } else {
        LOG_INFO("Socket succesfully configured as NON-BLOCKING");
    }
}

bool Socket::connect_sock(const std::string& target_ip, int target_port) {
    if (sock_fd == -1) return false;

    struct sockaddr_in target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(target_port);

    if (inet_pton(AF_INET, target_ip.c_str(), &target_addr.sin_addr) <= 0) {
        LOG_ERROR("Invalid proxy target IP address: " + target_ip);
        return false;
    }

    int res = connect(sock_fd, (struct sockaddr*)&target_addr, sizeof(target_addr));
    
    if (res < 0) {
        if (errno == EINPROGRESS) {
            LOG_DEBUG("Upstream connection to " + target_ip + " is in progress...");
            return true; 
        } else {
            LOG_ERROR("Upstream connection failed immediately: " + std::string(strerror(errno)));
            return false;
        }
    }

    LOG_INFO("Upstream connection established instantly to " + target_ip);
    return true;
}
