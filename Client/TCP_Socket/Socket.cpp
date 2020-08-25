//
// Created by giuseppetoscano on 13/06/20.
//

#include <fcntl.h>
#include "Socket.h"

#define SIZE 2048
#define MAXFD 50000

Socket::Socket(int sockfd): sockfd(sockfd), maxfd(MAXFD), timeout_secs(-1){
    // default timeout = -1 -> unlimited
    std::cout<<"Socket "<<sockfd<<" created"<<std::endl;
}

Socket::Socket(){
    sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    maxfd = MAXFD;
    timeout_secs = -1;
    if (sockfd < 0) throw std::runtime_error("Cannot create socket");
    std::cout<<"Socket "<<sockfd<<" created"<<std::endl;
}

Socket::~Socket(){
    if (sockfd != 0){
        std::cout<<"Socket "<<sockfd<<" closed"<<std::endl;
        close(sockfd);
    }
}

Socket::Socket(Socket &&other): sockfd(other.sockfd), maxfd(other.maxfd){
    timeout_secs = other.timeout_secs;
    timeout_usecs = other.timeout_usecs;
    other.sockfd = 0;
}

Socket & Socket::operator=(Socket &&other){
    if (sockfd != 0) close(sockfd);
    sockfd = other.sockfd;
    maxfd = other.maxfd;
    timeout_secs = other.timeout_secs;
    timeout_usecs = other.timeout_usecs;
    other.sockfd = 0;
    return *this;
}

ssize_t Socket::read(char *buffer, size_t len, int options){
    struct timeval tv;
    if(timeout_secs >= 0 && timeout_usecs >= 0) {
        tv.tv_sec = timeout_secs;
        tv.tv_usec = timeout_usecs;
    }
    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET(sockfd,&rfds);
    int select_ret = select(maxfd+1,&rfds,NULL,NULL,(timeout_secs >= 0)?&tv:NULL); // if timeout_secs < 0 -> wait forever
    if(select_ret == 0){
        // timeout expired
        throw std::runtime_error("connection timed out");
    }
    if(select_ret < 0){
        // some error accours
        throw std::runtime_error("error in select");
    }
    ssize_t res = recv(sockfd, buffer, len, options);
    if (res < 0) throw std::runtime_error("Cannot read from socket");
    return res;
}
ssize_t Socket::write(const char *buffer, size_t len, int options){
    struct timeval tv;
    if(timeout_secs >= 0 && timeout_usecs >= 0) {
        tv.tv_sec = timeout_secs;
        tv.tv_usec = timeout_usecs;
    }
    fd_set wfds;

    FD_ZERO(&wfds);
    FD_SET(sockfd,&wfds);

    int select_ret = select(maxfd+1,NULL,&wfds,NULL,(timeout_secs >= 0)?&tv:NULL); // if timeout_secs < 0 -> wait forever
    if(select_ret == 0){
        // timeout expired
        throw std::runtime_error("connection timed out");
    }
    if(select_ret < 0){
        // some error accours
        throw std::runtime_error("error in select");
    }
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

void Socket::setMaxfd(int maxfd) {
    Socket::maxfd = maxfd;
}

void Socket::setTimeoutSecs(int timeoutSecs) {
    timeout_secs = timeoutSecs;
}

void Socket::setTimeoutUsecs(int timeoutUsecs) {
    timeout_usecs = timeoutUsecs;
}
