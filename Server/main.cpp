/*
 *
 *
 * 
 */

#include <iostream>
#include <map>
#include <mutex>
#include <atomic>
#include <vector>
#include <exception>
#include <thread>
#include "Entities/File/File.h"
#include "Entities/Directory/Directory.h"
#include "Entities/SocketServer/SocketServer.h"
#include "Entities/Socket/Socket.h"

// Communication
#include "Usefull functions/Communication/Communication.h"
#include "Entities/Database/Database.h"
#include "Usefull functions/utilities.h"

// DB
#include <sqlite3.h>

#include <filesystem>

#include "Usefull functions/main_functions.h"
#include "Entities/Exceptions/MyExceptions.h"

#define PORT 5110
#define PATH_TO_DB "../DB/"

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

    pid_t pid;
    pid = fork();
    switch(pid){
        case -1: {
            throw std::runtime_error("cannot fork");
        }
        case 0: {
            std::freopen("../Log/log.txt","w+",stdout); // redirect stdout to file
            int id = 0;
            std::map<int, Socket> sockets;
            std::string root_path = "server_directory";

            while (true) {
                struct sockaddr_in addr;
                socklen_t len = sizeof(addr);
                std::cout << getTimestamp().str()<<" Waiting for incoming connections at port " << PORT << "..." << std::endl;
                Socket s = ss.accept(&addr, len);

                char name[16];
                if (inet_ntop(AF_INET, &addr.sin_addr, name, sizeof(name)) == nullptr) {
                    throw std::runtime_error("Cannot convert");
                }
                std::cout << "Got a connection from " << name << ":" << ntohs(addr.sin_port) << "\n";

                //sockets.insert(std::pair<int,Socket>(id,std::move(s)));
                addSocket(sockets,id,s);

                std::thread t([&root_path, &sockets, id](){
                    try {
                        std::shared_ptr<Directory> root;
                        std::string db_path;
                        std::map<std::string, std::shared_ptr<File>> files; // <path,File>
                        std::map<std::string, std::shared_ptr<Directory>> dirs; // <path, Directory>
                        std::string userDirPath, username, password, mode;
                        // SYNC 'client'
                        while (rcvConnectRequest(sockets[id], root_path, username, password, mode, root, files, dirs) != 0) {
                            std::cout << "Wrong username and/or password\n" << std::endl;
                        }

                        db_path = PATH_TO_DB + username + ".db";
                        userDirPath = root_path + "/" + username;

                        std::string msg;
                        msg = rcvMsg(sockets[id]);
                        if (msg == "GET-DB") { // client asks for server.db database version
                            sendFile(sockets[id], db_path, cleanPath(db_path, "../DB"));
                        } else if (msg == "Database up to date") {
                            //OK
                            sendMsg(sockets[id], "server_db_ok");
                        } else if (msg == "restore") {
                            //restore routine
                            restore(sockets[id], userDirPath, files, dirs);
                        } else {
                            // error
                            throw general_exception("unknown-message");
                        }while (true) {
                            msg = rcvMsg(sockets[id]);
                            if (msg == "update completed") {
                                // update completed, close the socket
                                // sockets.erase(id);
                                eraseSocket(sockets, id);
                                return;
                            }
                            manageModification(sockets[id], msg, db_path, userDirPath, files, dirs);
                        }
                    }catch(std::exception& e){
                        std::cout<<e.what()<<std::endl;
                        eraseSocket(sockets,id);
                    }
                });
                t.detach();
                id++; // increment 'id' for next socket in vector
            }
        }
        default: {
            std::cout<<"child: "<<pid<<std::endl;
        }
    }
    return 0;
}
