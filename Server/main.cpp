/*
 * Versione funzionante ma senza mutex su usersSocket, potrebbe dare problemi
 * Aggiungere anche un mutex sulle stampe
 * 
 */

#include <iostream>
#include <map>
#include <mutex>
#include <atomic>
#include <vector>
#include <thread>
#include "./FileManager/File.h"
#include "./FileManager/Directory.h"
#include "./TCP_Socket/SocketServer.h"
#include "./TCP_Socket/Socket.h"

// Communication
#include "Communication/Communication.h"

// DB
#include <sqlite3.h>

#define PORT 5076
#define MAXFD 50000

ServerSocket ss(PORT);

int main() {
    File f;
    File f2 {};
    Directory d;
    pthread_t threads[100];

    std::shared_ptr<Directory> my_root = Directory::getRoot();
    my_root->addFile("file.txt", "AAA");
    std::shared_ptr<Directory> dir = my_root->addDirectory("prova5");
    std::shared_ptr<File> file = dir->addFile("file3.txt", "BBB");
    dir->addDirectory("prova7");
    my_root->addDirectory("prova6");
    if (dir->removeDir("prova7"))
        std::cout<<"Cartella cancellata correttamente"<<std::endl;
    else
        std::cout<<"Problema nel cancellare la cartella"<<std::endl;

    my_root->ls(4);

    if (my_root->renameDir("prova5", "provaRename"))
        std::cout<<"Cartella rinominata correttamente"<<std::endl;
    else
        std::cout<<"Problema nel rinominare la cartella"<<std::endl;

    if (dir->renameFile("file3.txt", "fileRename.txt"))
        std::cout<<"File rinominato correttamente"<<std::endl;
    else
        std::cout<<"Problema nel rinominare il file"<<std::endl;

    my_root->ls(4);

    while (true){

        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        std::cout<<"Waiting for incoming connections at port "<<PORT<<"..."<<std::endl;
        Socket s = ss.accept(&addr, len);
        std::string username;

        char name[16];
        if (inet_ntop(AF_INET, &addr.sin_addr, name, sizeof(name)) == nullptr) throw std::runtime_error("Cannot convert");
        std::cout<<"Got a connection from "<<name<<":"<<ntohs(addr.sin_port)<<"\n";

        // SYNC 'client'
        if(rcvSyncRequest(s,username) != 0){
            std::cout<<"Errore\n";
        }
        std::string msg;
        msg = rcvMsg(s);
        if(msg == "GET-DB"){ // client asks for server.db database version
            sendFile(s,"../DB/"+username+".db");
        } else if(msg == "Database up to date"){
            //ok
        } else {
            // error
        }
        while(1) {
            msg = rcvMsg(s);
            if(msg.find("FILE") == 0){
                // file modification handler
            } else if(msg.find("DIR") == 0){
                //dirs modification handler
            } else {
                std::cout<<"sono in else"<<std::endl;
                //error
                //return -1;
            }
            sendMsg(s,"OK da server");
        }


        //TEST DIR 'path'
        //std::cout<<"Stringa ricevuta dal client: "<<s.rcvDir()<<std::endl;
        //s.sendMsg("OK");

        //TEST FILE 'path'
        //s.rcvFile("./server_directory/file.txt");
    }

    /*f.setDigest(computeDigest("prova.txt"));
    f2.setDigest(computeDigest("prova2.txt"));

    compareDigests(f.getDigest(), f.getDigest());
    compareDigests(f.getDigest(), f2.getDigest());*/


}
