#include <iostream>
#include "server.hpp"
#include <csignal>
#include <atomic>
#include <string.h>

std::atomic<bool> server_running(true);

void handle_signal(int signum) {
    std::cout<<"\n[System] Caught signal " << signum << ". Initiating graceful shutdown..." << std::endl;
    server_running.store(false);
}

int main(){

    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handle_signal;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    std::cout << "Booting web server..." << std::endl;

    Server my_server_(8000);

    my_server_.start_server(server_running);


    std::cout << "Server safely powered down. Goodbye!" << std::endl;
    return 0;

}