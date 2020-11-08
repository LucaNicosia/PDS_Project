#include <iostream>
#include <map>
#include <mutex>
#include <exception>
#include <thread>
#include "Entities/File/File.h"
#include "Entities/Directory/Directory.h"
#include "Entities/SocketServer/SocketServer.h"
#include "Entities/Socket/Socket.h"

#include "Usefull functions/Communication/Communication.h"
#include "Usefull functions/utilities.h"

#include "Usefull functions/main_functions.h"
#include "Entities/Exceptions/MyExceptions.h"

int main(int argc, char** argv){

    if(argc != 2)
        throw std::runtime_error("WRONG USAGE: server PORT");
    int port = atoi(argv[1]);

    pid_t pid;
    pid = fork();
    switch(pid){
        case -1: {
            throw std::runtime_error("cannot fork");
        }
        case 0: {
            ServerSocket ss(port);
            std::mutex socket_mutex;
            std::mutex users_mutex;
            Log_Writer.setLogFilePath(LOG_PATH);
            Log_Writer.setUseMutex(true);
            std::ostringstream os;
            int id = 0;
            std::map<int, Socket> sockets; // <socket_id, Socket>
            // users_connected: mainly used for checking users already connected and maybe usefull for future usage
            std::map<std::string,int> users_connected; //<username, socket_id>
            std::string root_path = SERVER_DIRECTORY;

            while (true) {
                struct sockaddr_in addr;
                socklen_t len = sizeof(addr);
                std::mutex user_db_mutex;
                os << "Waiting for incoming connections at port " << port << "...";
                Log_Writer.writeLogAndClear(os);
                Socket s = ss.accept(&addr, len);
                char name[16];
                if (inet_ntop(AF_INET, &addr.sin_addr, name, sizeof(name)) == nullptr) {
                    throw std::runtime_error("Cannot convert");
                }
                os << "Got a connection from " << name << ":" << ntohs(addr.sin_port);
                Log_Writer.writeLogAndClear(os);

                addSocket(sockets,id,s,socket_mutex);

                std::thread t([&root_path, &sockets, id, &users_connected, &os, &users_mutex, &socket_mutex, &user_db_mutex](){
                    try
                    {
                        sockets[id].setTimeoutSecs(60); // set timeout on socket
                        sockets[id].setTimeoutUsecs(0);
                        std::shared_ptr<Directory> root;
                        std::string db_path;
                        std::map<std::string, std::shared_ptr<File>> files; // <path,File>
                        std::map<std::string, std::shared_ptr<Directory>> dirs; // <path, Directory>
                        std::string userDirPath, username, password, mode;
                        // SYNC 'client'
                        int rc;
                        while ((rc=rcvConnectRequest(sockets[id], root_path, username, password, mode, root, files, dirs, users_connected, users_mutex, id, user_db_mutex)) < 0) {
                            switch(rc){
                                case -1:
                                    os << "Wrong username and/or password";
                                    break;
                                case -2:
                                    os << "User "+username+" already connected";
                                    break;
                                default:
                                    os << "unknown error type";
                            }
                            Log_Writer.writeLogAndClear(os);
                            throw general_exception(os.str());
                        }

                        db_path = PATH_TO_DB + username + ".db";
                        userDirPath = root_path + "/" + username;

                        std::string msg;
                        msg = rcvMsg(sockets[id]);
                        if (msg == "GET-DB") { // Client asks for server.db database version
                            sendFile(sockets[id], db_path, cleanPath(db_path, "../DB"));
                        } else if (msg == "Database up to date") {
                            // OK
                            sendMsg(sockets[id], "server_db_ok");
                        } else if (msg == "restore") {
                            // Restore routine
                            restore(sockets[id], userDirPath, files, dirs);
                        } else {
                            // Error
                            throw general_exception("unknown-message");
                        }
                        while (true) {
                            msg = rcvMsg(sockets[id]);
                            if (msg == "update completed") {
                                // Update completed, close the socket
                                eraseSocket(sockets, id, socket_mutex);
                                eraseUser(users_connected,username,users_mutex);
                                return;
                            }
                            manageModification(sockets[id], msg, db_path, userDirPath, files, dirs);
                        }
                    }catch(std::exception& e){
                        std::ostringstream error;
                        error << e.what();
                        Log_Writer.writeLog(error);
                        eraseSocket(sockets,id,socket_mutex);
                        std::lock_guard<std::mutex>lg(users_mutex);
                        for(auto it=users_connected.begin();it!=users_connected.end();++it){
                            if(it->second == id){ // Find username by socket id
                                users_connected.erase(it->first);
                                break;
                            }
                        }
                    }
                });
                t.detach();
                id++; // Increment 'id' for next socket in vector
            }
        }
        default: {
            std::cout<<"child: "<<pid<<std::endl;
        }
    }
    return 0;
}
