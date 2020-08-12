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

// Communication
#include "Communication/Communication.cpp"

// DB
#include <sqlite3.h>

#define PORT 5073
#define MAXFD 50000

ServerSocket ss(PORT);

int main() {
    File f;
    File f2 {};
    Directory d;
    pthread_t threads[100];
    while (true){

        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        std::cout<<"Waiting for incoming connections at port "<<PORT<<"..."<<std::endl;
        Socket s = ss.accept(&addr, len);

        char name[16];
        if (inet_ntop(AF_INET, &addr.sin_addr, name, sizeof(name)) == nullptr) throw std::runtime_error("Cannot convert");
        std::cout<<"Got a connection from "<<name<<":"<<ntohs(addr.sin_port)<<"\n";

        //TEST SYNC 'client'
        rcvSyncRequest(s);
        while(1) {
            rcvMsg(s);
            sendMsg(s,"OK");
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
