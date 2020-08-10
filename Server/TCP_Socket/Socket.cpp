//
// Created by giuseppetoscano on 13/06/20.
//

#include <fcntl.h>
#include "Socket.h"

#define SIZE 1024

Socket::Socket(int sockfd): sockfd(sockfd){
    std::cout<<"Socket "<<sockfd<<" created"<<std::endl;
}

Socket::Socket(){
    sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) throw std::runtime_error("Cannot create socket");
    std::cout<<"Socket "<<sockfd<<" created"<<std::endl;
}

Socket::~Socket(){
    if (sockfd != 0){
        std::cout<<"Socket "<<sockfd<<" closed"<<std::endl;
        close(sockfd);
    }
}

Socket::Socket(Socket &&other): sockfd(other.sockfd){
    other.sockfd = 0;
}

Socket & Socket::operator=(Socket &&other){
    if (sockfd != 0) close(sockfd);
    sockfd = other.sockfd;
    other.sockfd = 0;
    return *this;
}

ssize_t Socket::read(char *buffer, size_t len, int options){
    ssize_t res = recv(sockfd, buffer, len, options);
    if (res < 0) throw std::runtime_error("Cannot read from socket");
    return res;
}
ssize_t Socket::write(const char *buffer, size_t len, int options){
    ssize_t res = send(sockfd, buffer, len, options);
    if (res < 0) throw std::runtime_error("Cannot write to socket");
    return res;
}
void Socket::connect(struct sockaddr_in *addr, unsigned int len){
    if (::connect(sockfd, reinterpret_cast<struct sockaddr*>(addr), len) != 0)
        throw std::runtime_error("Cannot connect to remote socket");
}

void Socket::inizialize_and_connect(in_port_t port, sa_family_t family, const std::string& address){
    struct sockaddr_in addr;
    unsigned int len = sizeof(addr);


    addr.sin_family = family;
    addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(family, address.c_str(), &addr.sin_addr)<=0){
        printf("\nInvalid address/ Address not supported \n");
        //return -1; // Exception
    }

    this->connect(&addr, len);
}
