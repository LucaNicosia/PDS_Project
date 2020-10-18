//
// Created by giuseppetoscano on 10/08/20.
//

#ifndef PDS_PROJECT_SERVER_COMMUNICATION_H
#define PDS_PROJECT_SERVER_COMMUNICATION_H

#include "../../Entities/Socket/Socket.h"
#include "../Crypto/MyCryptoLibrary.h"
#include <fcntl.h>
#include <sstream>
#include "../../Entities/Exceptions/MyExceptions.h"

#define SIZE 2048
#define DEBUG 1

int sendMsg(Socket& s, const std::string msg);
std::string rcvMsg(Socket& s);
int rcvFile(Socket& s, const std::string path);
int sendFile(Socket& s, const std::string path, const std::string path_to_send);



//COMMUNICATION BETWEEN CLIENT AND SERVER
int sendMsg(Socket& s, const std::string msg){
    try {
        if(DEBUG)
            std::cout << "Stringa Mandata: " << std::string(msg) << " msg-size: " << msg.size() << " sul socket "<< s.__sock_fd() << std::endl;
        int ret = s.write(msg.c_str(), msg.size(), 0);
        if (ret == 0)
            throw socket_exception("sending empty message");
        return ret;
    }
    catch(socket_exception& e){
        throw e;
    }
    catch(std::exception& e){
        throw socket_exception(e.what());
    }
}

std::string rcvMsg(Socket& s){
    try {
        char msg[SIZE];
        int size = s.read(msg, SIZE, 0);
        if (size == 0)
            throw socket_exception("empty message received");
        msg[size] = '\0';
        if(DEBUG)
            std::cout << "Stringa ricevuta: " << std::string(msg) << " msg-size: " << size << " sul socket "<< s.__sock_fd() << std::endl;
        return std::string(msg);
    }catch(socket_exception& e){
        throw e;
    }
    catch(std::exception& e){
        throw socket_exception(e.what());
    }
};

int rcvFile(Socket& s, const std::string path){
    std::string fileData = rcvMsg(s); // FILE <path> <length>
    unsigned long long int length = std::stoll(fileData.substr(fileData.find_last_of(" ")));
    sendMsg(s, "OK");
    int rec;
    char buf [SIZE];
    int to;
    to=creat(path.c_str(),0777);
    if(to<0){
        throw filesystem_exception("Error creating destination file at "+path);
    }
    while(length > 0){
        rec = s.read(buf,sizeof(buf),0);
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
    unsigned long long int length = myFile.tellg();
    myFile.close();
    sendMsg(s, std::string ("FILE "+path_to_send+" "+std::to_string(length)));
    std::string msg = rcvMsg(s);
    if(msg == "OKDONE" && length == 0) {
        // no data to tranfer,return
        return 1;
    }
    if(msg != "OK"){
        throw general_exception("error in sendFile");
    }
    //...file transfer...
    int from;
    from=open(path.c_str(),O_RDONLY);
    if(from<0){
        throw filesystem_exception("error in opening file");
    }
    int size;
    int x;
    char buf [SIZE];
    while((size=::read(from,buf,sizeof(buf)))!=0) {
        x = s.write(buf, size, 0);
    }
    close(from);
    return 0;
};

#endif //PDS_PROJECT_SERVER_COMMUNICATION_H