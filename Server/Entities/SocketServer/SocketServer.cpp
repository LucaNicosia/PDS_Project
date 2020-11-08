#include "SocketServer.h"

ServerSocket::ServerSocket(int port){
    struct sockaddr_in sockaddrIn;
    sockaddrIn.sin_port = htons(port);
    sockaddrIn.sin_family = AF_INET;
    sockaddrIn.sin_addr.s_addr = htonl(INADDR_ANY);

    if (::bind(sockfd, reinterpret_cast<struct sockaddr *>(&sockaddrIn), sizeof(sockaddrIn)) != 0)
        throw std::runtime_error("Cannot bind port ");
    if (::listen(sockfd, 0) != 0)
        throw std::runtime_error("Cannot listen");
}

Socket ServerSocket::accept(struct sockaddr_in* addr, socklen_t len) {
    int fd = ::accept(sockfd, reinterpret_cast<struct sockaddr *>(addr), &len);
    if (fd < 0) throw std::runtime_error("Cannot accept socket");
    return Socket(fd);
}