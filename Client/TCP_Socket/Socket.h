//
// Created by giuseppetoscano on 13/06/20.
//

#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <stdexcept>
#include <sys/socket.h>
#include <iostream>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

class Socket {
    int sockfd;
    int maxfd;
    int timeout_secs;
    int timeout_usecs;

     Socket(int sockfd);

     Socket(const Socket&) = delete;
     Socket &operator=(const Socket&) = delete;

 public:
     Socket();
     ~Socket();
     Socket(Socket &&other);
     Socket &operator=(Socket &&other);
     ssize_t read(char *buffer, size_t len, int options);
     ssize_t write(const char *buffer, size_t len, int options);
     void connect(struct sockaddr_in *addr, unsigned int len);
     void inizialize_and_connect(in_port_t port, sa_family_t family, const std::string& address);

    void setMaxfd(int maxfd);

    void setTimeoutSecs(int timeoutSecs);

    void setTimeoutUsecs(int timeoutUsecs);

     int __sock_fd(){ return sockfd; }

     friend class ServerSocket;

};

#endif