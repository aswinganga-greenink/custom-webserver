#pragma once

#include <atomic>
#include <iostream>

#include "socket.hpp"
#include "threadpool.hpp"
#include "config.hpp"

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