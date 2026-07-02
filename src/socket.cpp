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

Socket::Socket() : port(0), sock_fd(-1) {}

Socket::Socket(int port) : port(port), sock_fd(-1) {}

Socket::Socket(Socket&& other) {
    this->port    = other.port;
    this->sock_fd = other.sock_fd;
    other.sock_fd = -1;
    other.port    = 0;
}

int Socket::release_fd() {
    int fd  = sock_fd;
    sock_fd = -1;
    return fd;
}

Socket::~Socket() {
    if (sock_fd != -1) {
        close(sock_fd);
    }
}

void Socket::bind_sock() {
    struct addrinfo hints, *res, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;  // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;  // For binding

    std::string port_str = std::to_string(port);
    int         status   = getaddrinfo(NULL, port_str.c_str(), &hints, &res);
    if (status != 0) {
        LOG_ERROR("getaddrinfo error: " + std::string(gai_strerror(status)));
        exit(EXIT_FAILURE);
    }

    for (p = res; p != NULL; p = p->ai_next) {
        sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock_fd == -1) continue;

        int opt = 1;
        setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        if (is_non_blocking_flag) {
            int flags = fcntl(sock_fd, F_GETFL, 0);
            if (flags != -1) fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);
        }

        if (bind(sock_fd, p->ai_addr, p->ai_addrlen) == 0) {
            break;  // Successfully bound
        }

        close(sock_fd);
        sock_fd = -1;
    }

    freeaddrinfo(res);

    if (p == NULL) {
        LOG_ERROR("CRITICAL ERROR: Failed to bind to port " + std::to_string(port));
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
    struct sockaddr_storage client_addr;
    socklen_t               client_size = sizeof(client_addr);
    int client_fd = accept(sock_fd, (struct sockaddr*)&client_addr, &client_size);
    return client_fd;
}

void Socket::set_non_blocking() {
    is_non_blocking_flag = true;
    if (sock_fd == -1) return;

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
    struct addrinfo hints, *res, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    std::string port_str = std::to_string(target_port);
    int         status   = getaddrinfo(target_ip.c_str(), port_str.c_str(), &hints, &res);
    if (status != 0) {
        LOG_ERROR("getaddrinfo error for proxy target: " + std::string(gai_strerror(status)));
        return false;
    }

    bool connected = false;
    for (p = res; p != NULL; p = p->ai_next) {
        sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock_fd == -1) continue;

        if (is_non_blocking_flag) {
            int flags = fcntl(sock_fd, F_GETFL, 0);
            if (flags != -1) fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);
        }

        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
            if (errno == EINPROGRESS || errno == EWOULDBLOCK) {
                connected = true;
                LOG_DEBUG("Upstream connection in progress to " + target_ip);
                break;
            }
            close(sock_fd);
            sock_fd = -1;
        } else {
            connected = true;
            LOG_INFO("Upstream connection established instantly to " + target_ip);
            break;
        }
    }

    freeaddrinfo(res);
    return connected;
}
