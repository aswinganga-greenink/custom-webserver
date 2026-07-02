#include "httphandler.hpp"

#include <errno.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "httprequest.hpp"
#include "logger.hpp"
#include "socket.hpp"

HttpHandler::HttpHandler(const ConfigParser& config) : config(config) {}

void HttpHandler::process_client(Session* session, OnCompleteCallback on_complete) {
    if (!session) {
        on_complete(false);
        return;
    }

    int  client_fd = session->client_fd;
    char buffer[4096];

    while (true) {
        int bytes_read = read(client_fd, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            session->request_buffer.append(buffer, bytes_read);
        } else if (bytes_read == 0) {
            break;  // Client closed connection
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;  // Exhausted available data
            }
            LOG_ERROR("Read error on FD: " + std::to_string(client_fd));
            on_complete(false);
            return;
        }
    }

    if (session->request_buffer.empty()) {
        on_complete(false);
        return;
    }

    if (session->request_buffer.find("\r\n\r\n") == std::string::npos) {
        // Request not complete, keep alive to get more data
        on_complete(true);
        return;
    }

    HttpRequest req;
    if (!req.parse(session->request_buffer)) {
        LOG_WARN("Recieved malinformed HTTP request. Dropping connection.");
        on_complete(false);
        return;
    }

    if (!config.proxy_route.empty() && req.uri.find(config.proxy_route) == 0) {
        LOG_INFO("Proxy route matched for URI: " + req.uri);
        session->state        = ProxyState::CONNECTING_TO_BACKEND;
        session->original_req = req;

        Socket upstream;
        upstream.set_non_blocking();
        upstream.connect_sock(config.proxy_target_ip, config.proxy_target_port);

        session->upstream_fd = upstream.release_fd();
        on_complete(true);
        return;
    }

    session->request_buffer.clear();
    LOG_INFO("Parsed " + req.method + " request for " + req.uri);

    bool keep_alive = false;
    if (req.headers.count("Connection") && req.headers["Connection"] == "keep-alive") {
        keep_alive = true;
        LOG_DEBUG("Browser requested Keep-Alive on FD " + std::to_string(client_fd));
    }

    std::string response = build_response(req.uri, keep_alive);
    write(client_fd, response.c_str(), response.size());

    on_complete(keep_alive);
}

std::string HttpHandler::build_response(const std::string& uri, bool keep_alive) {
    std::filesystem::path base_dir;

    try {
        base_dir = std::filesystem::canonical(config.document_root);
    } catch (const std::exception& e) {
        LOG_ERROR("CRITICAL: Document root directory : " + config.document_root +
                  "directory is missing!");
        return "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n";
    }

    std::string safe_uri = uri;
    if (!safe_uri.empty() && safe_uri[0] == '/') {
        safe_uri = safe_uri.substr(1);
    }
    if (safe_uri.empty()) {
        safe_uri = "index.html";
    }

    std::filesystem::path target_path   = base_dir / safe_uri;
    std::filesystem::path resolved_path = std::filesystem::weakly_canonical(target_path);

    if (resolved_path.string().find(base_dir.string()) != 0) {
        LOG_WARN("SECURITY ALERT: Directory traversal blocked -> " + uri);
        std::string error_body = "<html><body><h1>403 Forbidden</h1></body></html>";
        return "HTTP/1.1 403 Forbidden\r\n"
               "Content-Type: text/html\r\n"
               "Content-Length: " +
               std::to_string(error_body.length()) +
               "\r\n"
               "Connection: close\r\n\r\n" +
               error_body;
    }

    std::ifstream file(resolved_path, std::ios::binary);

    if (file.is_open()) {
        std::stringstream buffer_stream;
        buffer_stream << file.rdbuf();
        std::string content = buffer_stream.str();

        std::string mime_type = get_mime_type(resolved_path.string());

        std::string conn_header = keep_alive ? "keep-alive" : "close";

        return "HTTP/1.1 200 OK\r\n"
               "Content-Type: " +
               mime_type +
               "\r\n"
               "Content-Length: " +
               std::to_string(content.length()) +
               "\r\n"
               "Connection: " +
               conn_header + "\r\n\r\n" + content;
    }

    LOG_ERROR("404 Not Found -> " + resolved_path.string());
    std::string error_body =
        "<html><body style='font-family:monospace;'><h1>404 File Not Found</h1></body></html>";

    return "HTTP/1.1 404 Not Found\r\n"
           "Content-Type: text/html\r\n"
           "Content-Length: " +
           std::to_string(error_body.length()) +
           "\r\n"
           "Connection: close\r\n\r\n" +
           error_body;
}

std::string HttpHandler::extract_path(const std::string& raw_request) {
    std::istringstream stream(raw_request);
    std::string        method, path, protocol;

    stream >> method >> path >> protocol;

    if (path == "/") {
        path = "/index.html";
    }

    return "public" + path;
}

std::string HttpHandler::get_mime_type(const std::string& filepath) {
    if (filepath.find(".html") != std::string::npos) return "text/html";
    if (filepath.find(".css") != std::string::npos) return "text/css";
    if (filepath.find(".js") != std::string::npos) return "application/javascript";
    if (filepath.find(".png") != std::string::npos) return "image/png";
    if (filepath.find(".jpg") != std::string::npos || filepath.find(".jpeg") != std::string::npos)
        return "image/jpeg";
    if (filepath.find(".ico") != std::string::npos) return "image/x-icon";

    return "application/octet-stream";
}

void HttpHandler::process_proxy(Session* session, int ready_fd, OnCompleteCallback on_complete) {
    if (!session) {
        on_complete(false);
        return;
    }

    if (session->state == ProxyState::CONNECTING_TO_BACKEND) {
        int bytes_written = write(session->upstream_fd, session->request_buffer.c_str(),
                                  session->request_buffer.size());
        if (bytes_written < 0) {
            LOG_ERROR("Failed to write to upstream socket on FD: " +
                      std::to_string(session->upstream_fd));
            on_complete(false);
            return;
        }

        session->request_buffer.erase(0, bytes_written);
        if (session->request_buffer.empty()) {
            session->state = ProxyState::READING_FROM_BACKEND;
        }

        on_complete(true);
    } else if (session->state == ProxyState::READING_FROM_BACKEND) {
        char buffer[4096];
        while (true) {
            int bytes_read = read(session->upstream_fd, buffer, sizeof(buffer));
            if (bytes_read > 0) {
                int w = write(session->client_fd, buffer, bytes_read);
                if (w < 0) {
                    LOG_ERROR("Failed to stream to client FD: " +
                              std::to_string(session->client_fd));
                    on_complete(false);
                    return;
                }
            } else if (bytes_read == 0) {
                session->state = ProxyState::COMPLETED;
                on_complete(false);  // Finished proxying
                return;
            } else {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                LOG_ERROR("Upstream read error on FD: " + std::to_string(session->upstream_fd));
                on_complete(false);
                return;
            }
        }
        on_complete(true);
    }
}