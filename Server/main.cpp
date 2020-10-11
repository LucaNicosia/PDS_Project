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
            std::map<int, Socket> sockets; // <socket_id, Socket>
            // users_connected: mainly used for checking users already connected and maybe usefull for future usage
            std::map<std::string,int> users_connected; //<username, socket_id>
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
                addSocket(sockets,id,s,m);

                std::thread t([&root_path, &sockets, id, &users_connected](){
                    try {
                        std::shared_ptr<Directory> root;
                        std::string db_path;
                        std::map<std::string, std::shared_ptr<File>> files; // <path,File>
                        std::map<std::string, std::shared_ptr<Directory>> dirs; // <path, Directory>
                        std::string userDirPath, username, password, mode;
                        // SYNC 'client'
                        int rc;
                        while ((rc=rcvConnectRequest(sockets[id], root_path, username, password, mode, root, files, dirs, users_connected, m)) < 0) {
                            std::string error;
                            switch(rc){
                                case -1:
                                    error = "Wrong username and/or password";
                                    break;
                                case -2:
                                    error = "User "+username+" already connected";
                                    break;
                                default:
                                    error = "unknown error type";
                            }
                            std::cout<<getTimestamp().str()<<error<<std::endl;
                        }

                        users_connected[username] = id; // add user to the map associated with his socket_id

                        for(auto it:users_connected){
                            std::cout<<it.first<<":"<<it.second<<std::endl;
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
                                eraseSocket(sockets, id, m);
                                eraseUser(users_connected,username,m);
                                return;
                            }
                            manageModification(sockets[id], msg, db_path, userDirPath, files, dirs);
                        }
                    }catch(std::exception& e){
                        std::cout<<e.what()<<std::endl;
                        eraseSocket(sockets,id,m);
                        std::lock_guard<std::mutex>lg(m);
                        for(auto it=users_connected.begin();it!=users_connected.end();++it){
                            if(it->second == id){ // find username by socket id
                                users_connected.erase(it->first);
                            }
                        }
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
