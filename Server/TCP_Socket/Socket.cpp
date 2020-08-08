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

int Socket::rcvFile(const char *path){

    std::cout<<"Stringa ricevuta dal client: "<<rcvMsg()<<std::endl;
    sendMsg("OK");
    int rec;
    char buf [1024];
    int to;
    to=creat(path,0777);
    if(to<0){
        std::cout<<"Error creating destination file\n";
        return 0;
    }
    int w;
    while(rec=read(buf,sizeof(buf),0)){
        //std::cout<<buf<<std::endl;
        if(rec<0){
            std::cout<<"Error receiving\n";
            return 0;
        }
        w=::write(to,buf,rec);
    }
};

int Socket::sendMsg(const std::string msg){
    std::cout<<"QUA"<<std::endl;
    return write(msg.c_str(), msg.size(), 0);
}

std::string Socket::rcvMsg(){
    char msg [SIZE];
    int size = read(msg, SIZE, 0);
    msg[size] = '\0';
    return std::string(msg);
};

std::string Socket::rcvDir(){
    return rcvMsg();
};