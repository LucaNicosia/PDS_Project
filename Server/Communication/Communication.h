//
// Created by giuseppetoscano on 10/08/20.
//

#ifndef PDS_PROJECT_SERVER_COMMUNICATION_H
#define PDS_PROJECT_SERVER_COMMUNICATION_H

#include "../TCP_Socket/Socket.h"
#include "../Crypto/MyCryptoLibrary.h"
//#include <boost/beast/core/string.hpp>
#include <fcntl.h>
#include <sstream>

#define SIZE 2048


//COMMUNICATION BETWEEN CLIENT AND SERVER
int sendMsg(Socket& s, const std::string msg){
    std::cout<<"Stringa Mandata: "<<std::string(msg)<<" msg-size: "<<msg.size()<<std::endl;
    return s.write(msg.c_str(), msg.size(), 0);
}

std::string rcvMsg(Socket& s){
    char msg [SIZE];
    int size = s.read(msg, SIZE, 0);
    msg[size] = '\0';
    std::cout<<"Stringa ricevuta: "<<std::string(msg)<<" msg-size: "<<size<<std::endl;
    return std::string(msg);
};

/*

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
        if(digest.find("DIGEST") == 0) // ok
            return std::string (digest.substr(digest.find(" ")+1));
        else
            return "SYNC-ERROR";
    }
}

int rcvSyncRequest(Socket& s, std::string& username) {

    std::string msg = rcvMsg(s);
    std::string delimiter = " ";
    std::string client = msg.substr(msg.find(delimiter)+1, msg.size());
    username = client;
    //std::cout<<"client: "<<client<<std::endl;
    std::string filePath = "../DB/"+client+".db";
    //std::cout<<"file path: -"<<filePath<<"-"<<std::endl;

    std::ifstream input(filePath);
    if (input.is_open()){
        sendMsg(s, "SYNC-OK");
        std::string msg = rcvMsg(s);
        if (msg == "SYNC-OK"){
            std::string digest = b64_encode(computeDigest(filePath));
            sendMsg(s,"DIGEST "+digest);
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
*/
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
        return 0;
    }
    int w,cont=0;
    std::cout<<"rcvFile length: "<<length<<"\n";
    while(length > 0){
        rec = s.read(buf,sizeof(buf),0);
        if(rec<0){
            std::cout<<"Error receiving\n";
            return 0;
        }
        length -= rec;
        //std::cout<<buf<<std::endl;
        cont++;
        if(cont==1){
            //sendMsg(s,"OK");
        }
        w=::write(to,buf,rec);
        //std::cout<<write<<std::endl;
    }
    return -1;
};


int sendFile(Socket& s, const std::string path, const std::string path_to_send){
    // <- FILE 'path'
    std::ifstream myFile(path,std::ios::in);
    myFile.seekg(0,myFile.end);
    int length = myFile.tellg();
    myFile.close();
    sendMsg(s, std::string ("FILE "+path_to_send+" "+std::to_string(length)));
    if(rcvMsg(s) != "OK"){
        //error
    }
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