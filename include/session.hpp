#pragma once
#include <string>

#include "httprequest.hpp"

enum class ProxyState {
    NONE,
    CONNECTING_TO_BACKEND,
    FORWARDING_REQUEST,
    READING_FROM_BACKEND,
    STREAMING_TO_CLIENT,
    COMPLETED
};

struct Session {
    int client_fd   = -1;
    int upstream_fd = -1;

    ProxyState state = ProxyState::NONE;

    HttpRequest original_req;

    std::string request_buffer;
    std::string response_buffer;
};