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
    if (res < 0) throw std::runtime_error("Cannot read form socket");
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

int Socket::sendMsg(const std::string msg){
    return write(msg.c_str(), msg.size(), 0);
}

std::string Socket::rcvMsg(){
    char msg [SIZE];
    int size = read(msg, SIZE, 0);
    msg[size] = '\0';
    return std::string(msg);
};

int Socket::syncRequest(const std::string client){
    // <- SYNC 'client'
    return sendMsg(std::string ("SYNC "+client));
}
int Socket::sendDir(const std::string path){
    // <- DIR 'path'
    return sendMsg(std::string ("DIR "+path));
}

int Socket::sendFile(const std::string path){
    // <- FILE 'path'
    sendMsg(std::string ("FILE "+path));

    //...file transfer...
    int from;
    from=open(path.c_str(),O_RDONLY);
    if(from<0){
        std::cout<<"Error opening file\n";
        return 0;
    }
    int size;
    int s;
    char buf [1024];
    while((size=::read(from,buf,sizeof(buf)))!=0) {
        s = write(buf, size, 0);
        if (s < 0) {
            std::cout << "Error sending\n";
            return 0;
        }
    }
};

