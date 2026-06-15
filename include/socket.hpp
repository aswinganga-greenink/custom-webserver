#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>


    class Socket{

        private:
            int port;
            int sock_fd;
            
        
        public:
            struct sockaddr_in server, client;

            Socket(int port);
            ~Socket();
            Socket(Socket&& other);
            Socket& operator=(const Socket& other) = delete;


            void set_content();
            void bind_sock();
            void listen_sock();
            int accept_sock();
            void connect_sock();


    };
