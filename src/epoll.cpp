#include "epoll.hpp"
#include "logger.hpp"
#include <unistd.h>
#include <string.h>
#include <errno.h>

Epoll::Epoll() {
    epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1){
        LOG_ERROR("Failed to create epoll instance :" + std::string(strerror(errno)));
    }
    else {
        LOG_INFO("Epoll instance created successfully, FD : " + std::to_string(epoll_fd));
    }
}

Epoll::~Epoll() {
    if (epoll_fd != -1) {
        close(epoll_fd);
        LOG_INFO("Epoll instance safely destroyed");
    }
}

bool Epoll::add_socket(int socket_fd, uint32_t events) {
    struct epoll_event event;
    event.events = events;
    event.data.fd = socket_fd;

    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1) {
        LOG_ERROR("Failed to add socket to epoll: " + std::string(strerror(errno)));
        return false;
    }
    return true;
}

bool Epoll::modify_socket(int socket_fd, uint32_t events){
    struct epoll_event event;
    event.events = events;
    event.data.fd = socket_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket_fd, &event) == -1){
        LOG_ERROR("Failed to modify socket in epoll: " + std::string(strerror(errno)));
        return false;
    }
    return true;
}

bool Epoll::remove_socket(int socket_fd){
    struct epoll_event event;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, &event) == -1){
        LOG_ERROR("Failed to remove socket from epoll: " + std::string(strerror(errno)));
        return false;
    }
    return true;
}

int Epoll::wait(std::vector<struct epoll_event>& events, int timeout_ms){
    int num_ready = epoll_wait(epoll_fd, events.data(), events.size(), timeout_ms);

    if (num_ready == -1 && errno != EINTR) {
        LOG_ERROR("epoll_wait failed " + std::string(strerror(errno)));
    }
    return num_ready;
}