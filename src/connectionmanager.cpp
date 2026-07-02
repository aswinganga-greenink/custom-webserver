#include "connectionmanager.hpp"

#include <unistd.h>

#include "logger.hpp"

size_t ConnectionManager::get_current_time_ms() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

void ConnectionManager::add_or_update_timer(int fd, size_t timeout_ms) {
    std::lock_guard<std::mutex> lock(manager_mutex);
    size_t                      expire_time = get_current_time_ms() + timeout_ms;

    active_connections[fd] = expire_time;

    timer_queue.push({fd, expire_time});
}

void ConnectionManager::close_expired_connections(Epoll& epoll_engine) {
    std::lock_guard<std::mutex> lock(manager_mutex);
    size_t                      current_time = get_current_time_ms();

    while (!timer_queue.empty()) {
        TimerNode top = timer_queue.top();

        if (top.expiration_time > current_time) {
            break;
        }

        if (active_connections.count(top.fd) && active_connections[top.fd] == top.expiration_time) {
            LOG_DEBUG("Keep-Alive timeout on FD " + std::to_string(top.fd) + ". Closing socket.");

            epoll_engine.remove_socket(top.fd);
            close(top.fd);
            active_connections.erase(top.fd);
            sessions.erase(top.fd);
        }

        timer_queue.pop();
    }
}

void ConnectionManager::remove_timer(int fd) {
    std::lock_guard<std::mutex> lock(manager_mutex);
    active_connections.erase(fd);
}

Session* ConnectionManager::get_session(int fd) {
    std::lock_guard<std::mutex> lock(manager_mutex);
    if (sessions.count(fd)) {
        return sessions[fd].get();
    }
    return nullptr;
}

void ConnectionManager::add_session(int fd) {
    std::lock_guard<std::mutex> lock(manager_mutex);
    auto s = std::make_unique<Session>();
    s->client_fd = fd;
    sessions[fd] = std::move(s);
}

void ConnectionManager::remove_session(int fd) {
    std::lock_guard<std::mutex> lock(manager_mutex);
    sessions.erase(fd);
    active_connections.erase(fd);
}