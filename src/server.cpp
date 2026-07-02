#include "server.hpp"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <atomic>
#include <functional>
#include <iostream>
#include <vector>

#include "connectionmanager.hpp"
#include "epoll.hpp"
#include "httphandler.hpp"
#include "logger.hpp"
#include "socket.hpp"

Server::Server(const ConfigParser& config)
    : port(config.port), sock(config.port), pool(config.worker_threads), config(config) {}

Server::~Server() { LOG_INFO("Have a nice day"); }

void Server::start_server(std::atomic<bool>& is_running) {
    LOG_INFO("Starting server on port: " + std::to_string(port));

    sock.bind_sock();
    sock.listen_sock();
    sock.set_non_blocking();

    LOG_INFO("Server is now running and waiting for connection.");

    Epoll             event_loop;
    ConnectionManager connection_manager;
    int               server_fd = sock.get_fd();

    if (!event_loop.add_socket(server_fd, EPOLLIN)) {
        LOG_ERROR("Fatal: Failed to register server socket with epoll.");
        return;
    }

    std::vector<struct epoll_event> ready_events(64);

    while (is_running.load()) {
        int num_events = event_loop.wait(ready_events, 10);

        connection_manager.close_expired_connections(event_loop);

        if (num_events < 0) {
            if (errno == EINTR) continue;
            break;
        }

        for (int i = 0; i < num_events; ++i) {
            int      current_fd = ready_events[i].data.fd;
            uint32_t flags      = ready_events[i].events;

            if ((flags & EPOLLERR) || (flags & EPOLLHUP)) {
                LOG_WARN("Socket error on FD: " + std::to_string(current_fd));
                close(current_fd);
                continue;
            }

            if (current_fd == server_fd) {
                while (true) {
                    int client_fd = sock.accept_sock();
                    if (client_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        LOG_ERROR("Accpet failed");
                        break;
                    }

                    int flags = fcntl(client_fd, F_GETFL, 0);
                    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

                    LOG_INFO("Client connected! FD: " + std::to_string(client_fd));

                    if (!event_loop.add_socket(client_fd, EPOLLIN | EPOLLONESHOT)) {
                        close(client_fd);
                    } else {
                        connection_manager.add_or_update_timer(client_fd);
                        connection_manager.add_session(client_fd);
                    }
                }
            }

            else {
                pool.enqueue_task([current_fd, &event_loop, &connection_manager, this]() {
                    LOG_INFO("Worker thread processing fd: " + std::to_string(current_fd));
                    HttpHandler handler(this->config);

                    bool     is_upstream = false;
                    Session* session     = connection_manager.get_session(current_fd);
                    if (!session) {
                        session = connection_manager.get_session_by_upstream(current_fd);
                        if (session) {
                            is_upstream = true;
                        }
                    }

                    if (!session) {
                        LOG_WARN("Session not found for fd " + std::to_string(current_fd));
                        close(current_fd);
                        return;
                    }

                    auto callback = [current_fd, session, is_upstream, &event_loop,
                                     &connection_manager](bool keep_alive) {
                        if (session && session->state == ProxyState::CONNECTING_TO_BACKEND &&
                            !is_upstream) {
                            connection_manager.map_upstream(current_fd, session->upstream_fd);
                            connection_manager.add_or_update_timer(current_fd);
                            event_loop.add_socket(session->upstream_fd, EPOLLOUT | EPOLLONESHOT);
                        } else if (session && session->state == ProxyState::READING_FROM_BACKEND &&
                                   is_upstream) {
                            if (keep_alive) {
                                connection_manager.add_or_update_timer(session->client_fd);
                                event_loop.modify_socket(session->upstream_fd,
                                                         EPOLLIN | EPOLLONESHOT);
                            } else {
                                int c_fd = session->client_fd;
                                int u_fd = session->upstream_fd;
                                connection_manager.remove_timer(c_fd);
                                connection_manager.remove_session(c_fd);
                                event_loop.remove_socket(c_fd);
                                event_loop.remove_socket(u_fd);
                                close(c_fd);
                                close(u_fd);
                            }
                        } else if (keep_alive && !is_upstream) {
                            connection_manager.add_or_update_timer(current_fd);
                            event_loop.modify_socket(current_fd, EPOLLIN | EPOLLONESHOT);
                        } else {
                            int c_fd = session->client_fd;
                            int u_fd = session->upstream_fd;
                            connection_manager.remove_timer(c_fd);
                            connection_manager.remove_session(c_fd);
                            event_loop.remove_socket(c_fd);
                            close(c_fd);
                            if (u_fd != -1) {
                                event_loop.remove_socket(u_fd);
                                close(u_fd);
                            }
                        }
                    };

                    if (is_upstream) {
                        handler.process_proxy(session, current_fd, callback);
                    } else {
                        handler.process_client(session, callback);
                    }
                });
            }
        }
    }
}