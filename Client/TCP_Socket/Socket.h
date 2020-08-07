//
// Created by giuseppetoscano on 13/06/20.
//

#ifndef LAB05_SOCKET_H
#define LAB05_SOCKET_H

#include <stdexcept>
#include <sys/socket.h>
#include <iostream>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

class Socket {
     int sockfd;

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

     int __sock_fd(){ return sockfd; }

    int sendFile(const char *filename);

     friend class ServerSocket;
};


#endif //LAB05_SOCKET_H
