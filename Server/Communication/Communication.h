//
// Created by giuseppetoscano on 10/08/20.
//

#ifndef PDS_PROJECT_SERVER_COMMUNICATION_H
#define PDS_PROJECT_SERVER_COMMUNICATION_H

#include "../TCP_Socket/Socket.h"
#include "../Crypto/MyCryptoLibrary.h"
//#include <boost/beast/core/string.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <fcntl.h>
#include <sstream>

#define SIZE 2048

std::string b64_encode(std::string digest){
    using namespace boost::archive::iterators;

    std::stringstream os;
    typedef
    insert_linebreaks<         // insert line breaks every 72 characters
            base64_from_binary<    // convert binary values to base64 characters
                    transform_width<   // retrieve 6 bit integers from a sequence of 8 bit bytes
                            const char *,
                            6,
                            8
                    >
            >
            ,72
    >
            base64_text; // compose all the above operations in to a new iterator

    std::copy(
            base64_text(digest.c_str()),
            base64_text(digest.c_str() + digest.size()),
            ostream_iterator<char>(os)
    );

    return os.str();
}



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


int sendFile(Socket& s, const std::string path){
    // <- FILE 'path'
    sendMsg(s, std::string ("FILE "+path));
    if(rcvMsg(s) != "OK"){
        //error
    }
    //...file transfer...
    int from;
    //std::ifstream myFile(path,std::ios::in);
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
        std::cout<<buf<<std::endl;
    }
    std::cout<<"fine"<<std::endl;
    return -1;
};

int rcvFile(Socket& s, const char *path){

    std::cout<<"Stringa ricevuta dal client: "<<rcvMsg(s)<<std::endl;
    sendMsg(s, "OK");
    int rec;
    char buf [SIZE];
    int to;
    to=creat(path,0777);
    if(to<0){
        std::cout<<"Error creating destination file\n";
        return 0;
    }
    int w,cont=0;
    while(rec=s.read(buf,sizeof(buf),0)){
        if(rec<0){
            std::cout<<"Error receiving\n";
            return 0;
        }
        //std::cout<<buf<<std::endl;
        cont++;
        if(cont==1){
            sendMsg(s,"OK");
        }
        w=::write(to,buf,rec);
        //std::cout<<write<<std::endl;
    }
    std::cout<<"fine"<<std::endl;
    return -1;
};

int sendDir(Socket& s, const std::string path){
    // <- DIR 'path'
    return sendMsg(s, std::string ("DIR "+path));
}

std::string rcvDir(Socket& s){
    return rcvMsg(s);
};

#endif //PDS_PROJECT_SERVER_COMMUNICATION_H