#pragma once
#include <sys/epoll.h>
#include <vector>

class Epoll {
    private:
        int epoll_fd;

    public:
        Epoll();
        ~Epoll();


        Epoll(const Epoll&) = delete;
        Epoll& operator=(const Epoll&) = delete;

        bool add_socket(int socket_fd, uint32_t events);
        bool modify_socket(int socket_fd, uint32_t events);
        bool remove_socket(int socket_fd);

        int wait(std::vector<struct epoll_event>& events, int timeout_ms = -1);
};