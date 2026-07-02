#pragma once
#include <string>
#include "httprequest.hpp"

enum class ProxyState {
    CONNECTING_TO_BACKEND,
    FORWARDING_REQUEST,
    READING_FROM_BACKEND,
    STREAMING_TO_CLIENT,
    COMPLETED
};

struct Session {
    int client_fd;
    int upstream_fd;

    ProxyState state;

    HttpRequest original_req;

    std::string request_buffer;
    std::string response_buffer;
};