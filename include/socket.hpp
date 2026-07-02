#pragma once

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#include <string>

class Socket {
   private:
    int  port;
    int  sock_fd;
    bool is_non_blocking_flag = false;

   public:
    Socket(int port);
    Socket();
    ~Socket();
    Socket(Socket&& other);
    Socket& operator=(const Socket& other) = delete;
    Socket(const Socket&)                  = delete;

    int  get_fd() const { return sock_fd; }
    int  release_fd();
    void bind_sock();
    void listen_sock();
    int  accept_sock();
    bool connect_sock(const std::string& target_ip, int target_port);
    void set_non_blocking();
};
