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
#include "./TCP_Socket/SocketServer.h"

// DB
#include <sqlite3.h>

#define PORT 5058
#define MAXFD 50000

ServerSocket ss(PORT);

int main() {
    pthread_t threads[100];
    while (true){

        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        std::cout<<"Waiting for incoming connections at port "<<PORT<<"..."<<std::endl;
        Socket s = ss.accept(&addr, len);

        char name[16];
        if (inet_ntop(AF_INET, &addr.sin_addr, name, sizeof(name)) == nullptr) throw std::runtime_error("Cannot convert");
        std::cout<<"Got a connection from "<<name<<":"<<ntohs(addr.sin_port)<<"\n";

        char buffer[1024];
    }
}
