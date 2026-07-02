#pragma once
#include <chrono>
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_map>

#include "epoll.hpp"
#include "session.hpp"

struct TimerNode {
    int    fd;
    size_t expiration_time;

    bool operator>(const TimerNode& other) const { return expiration_time > other.expiration_time; }
};

class ConnectionManager {
   private:
    std::priority_queue<TimerNode, std::vector<TimerNode>, std::greater<TimerNode>> timer_queue;
    std::unordered_map<int, size_t>                   active_connections;
    std::unordered_map<int, std::unique_ptr<Session>> sessions;
    std::unordered_map<int, int>                      upstream_to_client;
    std::mutex                                        manager_mutex;
    size_t                                            get_current_time_ms();

   public:
    void add_or_update_timer(int fd, size_t timeout_ms = 5000);
    void close_expired_connections(Epoll& epoll_engine);
    void remove_timer(int fd);

    Session* get_session(int fd);
    Session* get_session_by_upstream(int upstream_fd);
    void     add_session(int fd);
    void     remove_session(int fd);
    void     map_upstream(int client_fd, int upstream_fd);
};