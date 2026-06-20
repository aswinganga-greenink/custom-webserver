#include <string.h>

#include <atomic>
#include <csignal>
#include <iostream>

#include "logger.hpp"
#include "server.hpp"

std::atomic<bool> server_running(true);

void handle_signal(int signum) {
    LOG_WARN("Caught system signal. Initiating shutdown...");
    server_running.store(false);
}

int main() {
    Logger::set_level(LogLevel::INFO);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    LOG_INFO("Booting up the server...");

    ConfigParser config;
    if (!config.load_from_file("server.conf")) {
        LOG_WARN("Falling back to default compiled architecture.");
    }

    Logger::set_level(config.log_level);
    LOG_INFO("Telemetry engine calibrated. Current Log Level: " +
             Logger::level_to_string(config.log_level));

    Server my_server_(config);
    my_server_.start_server(server_running);

    LOG_INFO("Server safely powered down. Goodbye!");
    return 0;
}