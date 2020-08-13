//
// Created by giuseppetoscano on 10/08/20.
//

#ifndef PDS_PROJECT_CLIENT_COMMUNICATION_H
#define PDS_PROJECT_CLIENT_COMMUNICATION_H

#include "../TCP_Socket/Socket.h"
#include "../Crypto/MyCryptoLibrary.h"
#include <fcntl.h>
#define SIZE 1024

//COMMUNICATION BETWEEN CLIENT AND SERVER
int sendMsg(Socket& s, const std::string msg){
    std::cout<<"Stringa mandata: "<<msg<<std::endl;
    return s.write(msg.c_str(), msg.size(), 0);
}

std::string rcvMsg(Socket& s){
    char msg [SIZE];
    int size = s.read(msg, SIZE, 0);
    msg[size] = '\0';
    std::cout<<"Stringa ricevuta: "<<std::string(msg)<<std::endl;
    return std::string(msg);
};

std::string syncRequest(Socket& s, const std::string client){
    // <- SYNC 'client'
    sendMsg(s, std::string ("SYNC "+client));
    std::string msg = rcvMsg(s);
    if (msg == "SYNC-ERROR"){
        //SYNC-ERROR
        sendMsg(s, "SYNC-ERROR");
        return std::string("SYNC-ERROR");
    }else{
        //SYNC-OK
        sendMsg(s, "SYNC-OK");
        std::string digest = rcvMsg(s);
        return std::string (digest);
    }
}

int rcvSyncRequest(Socket& s) {

    std::string msg = rcvMsg(s);
    std::string delimiter = " ";
    std::string client = msg.substr(msg.find(delimiter)+1, msg.size());
    //std::cout<<"client: "<<client<<std::endl;
    std::string filePath = "./DB/"+client+".txt";
    //std::cout<<"file path: -"<<filePath<<"-"<<std::endl;

    std::ifstream input(filePath);
    if (input.is_open()){
        sendMsg(s, "SYNC-OK");
        std::string msg = rcvMsg(s);
        if (msg == "SYNC-OK"){
            sendMsg(s,computeDigest(filePath));
        }else{
            //ERRORE
            std::cout<<"ERRORE"<<std::endl;
        }
        return 0;
    }else{
        sendMsg(s, "SYNC-ERROR");
        return -1;
    }
}

int sendFile(Socket& s, const std::string path){
    // <- FILE 'path'
    sendMsg(s, std::string ("FILE "+path));

    //...file transfer...
    int from;
    from=open(path.c_str(),O_RDONLY);
    if(from<0){
        std::cout<<"Error opening file\n";
        return 0;
    }
    int size;
    int x;
    char buf [SIZE];
    while((size=::read(from,buf,sizeof(buf)))!=0) {
        x = s.write(buf, size, 0);
        if (x < 0) {
            std::cout << "Error sending\n";
            return 0;
        }
    }
    return -1;
};


int rcvFile(Socket& s, const char *path){

    std::cout<<"Stringa ricevuta dal client: "<<rcvMsg(s)<<std::endl;
    sendMsg(s, "OK");
    int rec;
    char buf [1024];
    int to;
    to=creat(path,0777);
    if(to<0){
        std::cout<<"Error creating destination file\n";
        return 0;
    }
    int w;
    while(rec=s.read(buf,sizeof(buf),0)){
        //std::cout<<buf<<std::endl;
        if(rec<0){
            std::cout<<"Error receiving\n";
            return 0;
        }
        w=::write(to,buf,rec);
    }
    return -1;
};

int sendDir(Socket& s, const std::string path){
    // <- DIR 'path'
    return sendMsg(s, std::string ("DIR "+path));
}

std::string rcvDir(Socket& s){
    return rcvMsg(s);
};

#endif //PDS_PROJECT_CLIENT_COMMUNICATION_H