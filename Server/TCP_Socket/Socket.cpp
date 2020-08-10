//
// Created by giuseppetoscano on 13/06/20.
//

#include <fcntl.h>
#include "Socket.h"
#include "../Crypto/MyCryptoLibrary.cpp"

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

//COMMUNICATION BETWEEN CLIENT AND SERVER
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

int Socket::syncRequest(const std::string client){
    // <- SYNC 'client'
    return sendMsg(std::string ("SYNC "+client));
}

int Socket::rcvSyncRequest() {

    std::string msg = rcvMsg();
    std::cout<<"Stringa ricevuta dal client: "<<msg<<std::endl;
    std::string delimiter = " ";
    std::string client = msg.substr(msg.find(delimiter)+1, msg.size());
    //std::cout<<"client: "<<client<<std::endl;
    std::string filePath = "./DB/"+client+".txt";
    //std::cout<<"file path: -"<<filePath<<"-"<<std::endl;
    sendMsg(computeDigest(filePath));
    return 0;
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
    char buf [SIZE];
    while((size=::read(from,buf,sizeof(buf)))!=0) {
        s = write(buf, size, 0);
        if (s < 0) {
            std::cout << "Error sending\n";
            return 0;
        }
    }
    return -1;
};


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
    return -1;
};

int Socket::sendDir(const std::string path){
    // <- DIR 'path'
    return sendMsg(std::string ("DIR "+path));
}

std::string Socket::rcvDir(){
    return rcvMsg();
};

bool Socket::compareDBDigest (const std::string dbPath){
    std::string digest = rcvMsg();
    std::cout<<"Stringa ricevuta dal server: "<<digest<<std::endl;
    return compareDigests(computeDigest("./DB/ciao.txt"), digest);
}