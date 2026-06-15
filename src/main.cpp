#include <iostream>
#include "server.hpp"
#include "logger.hpp"
#include <csignal>
#include <atomic>
#include <string.h>

std::atomic<bool> server_running(true);

void handle_signal(int signum) {
    LOG_WARN("Caught system signal. Initiating shutdown...");
    server_running.store(false);
}

int main(){

    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handle_signal;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    LOG_INFO("Booting up the server...");

    Server my_server_(8000);

    my_server_.start_server(server_running);


    LOG_INFO("Server safely powered down. Goodbye!");
    return 0;

}