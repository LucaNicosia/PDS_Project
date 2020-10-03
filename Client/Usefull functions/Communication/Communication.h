//
// Created by giuseppetoscano on 10/08/20.
//

#ifndef PDS_PROJECT_SERVER_COMMUNICATION_H
#define PDS_PROJECT_SERVER_COMMUNICATION_H

#include "../../Entities/Socket/Socket.h"
#include "../Crypto/MyCryptoLibrary.h"
#include <fcntl.h>
#include <sstream>

#define SIZE 2048

//COMMUNICATION BETWEEN CLIENT AND SERVER
int sendMsg(Socket& s, const std::string msg){
    std::cout<<"Stringa Mandata: "<<std::string(msg)<<" msg-size: "<<msg.size()<<" sul socket "<<s.__sock_fd()<<std::endl;
    return s.write(msg.c_str(), msg.size(), 0);
}

std::string rcvMsg(Socket& s){
    char msg [SIZE];
    int size = s.read(msg, SIZE, 0);
    msg[size] = '\0';
    std::cout<<"Stringa ricevuta: "<<std::string(msg)<<" msg-size: "<<size<<" sul socket "<<s.__sock_fd()<<std::endl;
    return std::string(msg);
};

int rcvFile(Socket& s, const std::string path){
    std::string fileData = rcvMsg(s); // FILE <path> <length>
    int length = std::stoi(fileData.substr(fileData.find_last_of(" ")));
    sendMsg(s, "OK");
    int rec;
    char buf [SIZE];
    int to;
    to=creat(path.c_str(),0777);
    if(to<0){
        std::cout<<"Error creating destination file at "<<path<<"\n";
        return -1;
    }
    while(length > 0){
        rec = s.read(buf,sizeof(buf),0);
        if(rec<0){
            std::cout<<"Error receiving\n";
            return -1;
        }
        length -= rec;
        ::write(to,buf,rec);
    }
    close(to);
    return 0;
};


int sendFile(Socket& s, const std::string path, const std::string path_to_send){
    // <- FILE 'path'
    std::ifstream myFile(path,std::ios::in);
    myFile.seekg(0,myFile.end);
    int length = myFile.tellg();
    myFile.close();
    sendMsg(s, std::string ("FILE "+path_to_send+" "+std::to_string(length)));
    std::string msg = rcvMsg(s);
    if(msg == "OKDONE" && length == 0) {
        // no data to tranfer,return
        return 1;
    }
    if(msg != "OK"){
        std::cout<<"error in sendFile\n";
        return -1;
    }
    //...file transfer...
    int from;
    from=open(path.c_str(),O_RDONLY);
    if(from<0){
        std::cout<<"Error opening file\n";
        return -1;
    }
    int size;
    int x;
    char buf [SIZE];
    while((size=::read(from,buf,sizeof(buf)))!=0) {
        x = s.write(buf, size, 0);
        if (x < 0) {
            std::cout << "Error sending\n";
            return -1;
        }
    }
    close(from);
    return 0;
};

int sendDir(Socket& s, const std::string path){
    // <- DIR 'path'
    return sendMsg(s, std::string ("DIR "+path));
}

std::string rcvDir(Socket& s){
    return rcvMsg(s);
};

#endif //PDS_PROJECT_SERVER_COMMUNICATION_H