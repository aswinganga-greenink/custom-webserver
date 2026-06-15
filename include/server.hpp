#pragma once

#include <atomic>
#include <iostream>

#include "socket.hpp"
#include "threadpool.hpp"

class Server {
   private:
    int        port;
    Socket     sock;
    ThreadPool pool;

   public:
    Server(int port);
    ~Server();

    void start_server(std::atomic<bool>& is_running);
};