#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "../../Entities/Socket/Socket.h"
#include "../Crypto/MyCryptoLibrary.h"
#include <fcntl.h>
#include <sstream>
#include "../../Entities/Exceptions/MyExceptions.h"
#include "../utilities.h"
#include "../constants.h"

#define SIZE 2048

int sendMsg(Socket& s, const std::string msg);
std::string rcvMsg(Socket& s);
std::string rcvFile(Socket& s, const std::string path);
int sendFile(Socket& s, const std::string path, const std::string path_to_send);



//COMMUNICATION BETWEEN CLIENT AND SERVER
int sendMsg(Socket& s, const std::string msg){
    try {
        std::ostringstream os;
        os << "sent string: " << std::string(msg) << " msg-size: " << msg.size() << " on socket "<< s.__sock_fd();
        if(DEBUG)
            std::cout << os.str();
        Log_Writer.writeLog(os);
        int ret = s.write(msg.c_str(), msg.size(), 0);
        if (ret == 0)
            throw socket_exception("sending empty message");
        return ret;
    }
    catch(std::exception& e){
        throw socket_exception(e.what());
    }
}

std::string rcvMsg(Socket& s){
    try {
        std::ostringstream os;
        char msg[SIZE];
        int size = s.read(msg, SIZE, 0);
        if (size == 0)
            throw socket_exception("empty message received");
        msg[size] = '\0';
        os << "received string: " << std::string(msg) << " msg-size: " << size << " on socket " << s.__sock_fd();
        if (DEBUG)
            std::cout << os.str();
        Log_Writer.writeLog(os);
        return std::string(msg);
    }
    catch(std::exception& e){
        throw socket_exception(e.what());
    }
};

std::string rcvFile(Socket& s, const std::string path){
    std::string fileData = rcvMsg(s); // FILE <path> <length>
    unsigned long long int length = std::stoll(fileData.substr(fileData.find_last_of(" ")));
    sendMsg(s, "OK");
    int rec;
    char buf [SIZE]="";
    int to;
    to=creat(path.c_str(),0777);
    if(to<0){
        throw filesystem_exception("Error creating destination file at "+path);
    }
    while(length > 0){
        rec = s.read(buf,sizeof(buf),0);
        length -= rec;
        ::write(to,buf,rec);
        appendDigest(buf,rec);
    }
    close(to);
    return getAppendedDigest();
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

#endif //COMMUNICATION_H