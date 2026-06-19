#pragma once
#include <unordered_map>
#include <queue>
#include <mutex>
#include <chrono>
#include "epoll.hpp"

struct TimerNode {
    int fd;
    size_t expiration_time;

    bool operator>(const TimerNode& other) const{
        return expiration_time > other.expiration_time;
    }
};

class ConnectionManager {
    private:
        std::priority_queue<TimerNode, std::vector<TimerNode>, std::greater<TimerNode>> timer_queue;
        std::unordered_map<int, size_t> active_connections;
        std::mutex manager_mutex;
        size_t get_current_time_ms();

    public:
        void add_or_update_timer(int fd, size_t timeout_ms = 5000);
        void close_expired_connections(Epoll& epoll_engine);
};