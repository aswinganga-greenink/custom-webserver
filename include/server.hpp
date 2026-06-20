#pragma once

#include <atomic>
#include <iostream>

#include "config.hpp"
#include "connectionmanager.hpp"
#include "epoll.hpp"
#include "httphandler.hpp"
#include "socket.hpp"
#include "threadpool.hpp"

class Server {
   private:
    int        port;
    Socket     sock;
    ThreadPool pool;

    std::string document_root;

   public:
    Server(const ConfigParser& config);
    ~Server();

    void start_server(std::atomic<bool>& is_running);
};