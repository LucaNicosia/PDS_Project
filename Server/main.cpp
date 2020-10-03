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
#include "Entities/File/File.h"
#include "Entities/Directory/Directory.h"
#include "Entities/SocketServer/SocketServer.h"
#include "Entities/Socket/Socket.h"

// Communication
#include "Usefull functions/Communication/Communication.h"
#include "Entities/Database/Database.h"

// DB
#include <sqlite3.h>

#include <filesystem>

#include "Usefull functions/main_functions.h"

#define PORT 5110

ServerSocket ss(PORT);
std::mutex m;

void eraseSocket(std::map<int,Socket>& sockets, int id){
    std::lock_guard<std::mutex>lg(m);
    sockets.erase(id);
}

void addSocket(std::map<int,Socket>& sockets, int id, Socket& s){
    std::lock_guard<std::mutex>lg(m);
    sockets[id] = std::move(s);
}

int main() {
    int id = 0;
    std::map<int, Socket> sockets;
    std::string root_path = "server_directory";

    while (true) {
//restart:
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        std::cout << "Waiting for incoming connections at port " << PORT << "..." << std::endl;
        Socket s = ss.accept(&addr, len);

        char name[16];
        if (inet_ntop(AF_INET, &addr.sin_addr, name, sizeof(name)) == nullptr)
            throw std::runtime_error("Cannot convert");
        std::cout << "Got a connection from " << name << ":" << ntohs(addr.sin_port) << "\n";

        //sockets.insert(std::pair<int,Socket>(id,std::move(s)));
        addSocket(sockets,id,s);

        std::thread t([&root_path, &sockets, id](){
            std::shared_ptr<Directory> root;
            std::string db_path;
            std::map<std::string, std::shared_ptr<File>> files; // <path,File>
            std::map<std::string, std::shared_ptr<Directory>> dirs; // <path, Directory>
            std::cout<<"Entered in thread with id: "<<id<<std::endl;
            std::string  userDirPath, username, password, mode;
            // SYNC 'client'
            int cont = 0;

            while(rcvConnectRequest(sockets[id], root_path,username, password, mode, root, files, dirs) != 0){
                std::cout<<"Wrong username and/or password\n"<<std::endl;
            }

            // old code for fetch mode
            std::cout<<"FETCH MODE"<<std::endl;
            /*
            while(rcvSyncRequest(sockets[id],username, root_path,root,files,dirs) != 0) {
                std::cout << "Errore in SYNC\n";
            }*/
            db_path = "../DB/" + username + ".db";
            userDirPath = root_path + "/" + username;

            std::string msg;
            msg = rcvMsg(sockets[id]);
            if (msg == "GET-DB") { // client asks for server.db database version
                if(sendFile(sockets[id], db_path, cleanPath(db_path,"../DB"))<0){
                    // error handling
                }
            } else if (msg == "Database up to date") {
                //OK
                sendMsg(sockets[id], "server_db_ok");
            } else if (msg == "restore") {
                    //restore routine
                    restore(sockets[id],userDirPath,files,dirs);
            } else {
                // error
                std::cout<<"error in main\n";
            }
            try {
                while (true) {
                    msg = rcvMsg(sockets[id]);
                    if(msg == "update completed"){
                        // update completed, close the socket
                        // sockets.erase(id);
                        eraseSocket(sockets,id);
                        return;
                    }
                    manageModification(sockets[id],msg,db_path,userDirPath,files,dirs);
                }
            } catch (std::exception& e) {
                std::cout<<"sono in catch\n";
                std::cout<<e.what()<<std::endl;
                //sockets.erase(id);
                eraseSocket(sockets,id);
                return;
            }
        });
        t.detach();
        id++; // increment 'id' for next socket in vector
    }
}
