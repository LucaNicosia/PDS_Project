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

#define MAX_THREADS 20

std::mutex n_threads_mutex;
int n_threads = 0;

void update_n_threads(const int value);
int get_n_threads();

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

                if(get_n_threads() >= MAX_THREADS){
                    os << "max threads number reached: try again";
                    sendMsg(s,"CONNECT-ERROR");
                    Log_Writer.writeLogAndClear(os);
                    continue;
                }

                addSocket(sockets,id,s,socket_mutex);

                // start a thread for current connection request
                std::thread t([&root_path, &sockets, id, &users_connected, &os, &users_mutex, &socket_mutex, &user_db_mutex](){
                    try
                    {
                        update_n_threads(1);
                        sockets[id].setTimeoutSecs(60); // set timeout on socket
                        sockets[id].setTimeoutUsecs(0);
                        std::shared_ptr<Directory> root;
                        std::string db_path;
                        std::map<std::string, std::shared_ptr<File>> files; // <path,File>
                        std::map<std::string, std::shared_ptr<Directory>> dirs; // <path, Directory>
                        std::string userDirPath, username, password, mode;
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
                        }
                        while (true) {
                            msg = rcvMsg(sockets[id]);
                            if (msg == "update completed") {
                                // update completed, close the socket
                                eraseSocket(sockets, id, socket_mutex);
                                eraseUser(users_connected,username,users_mutex);
                                update_n_threads(-1);
                                return;
                            }
                            manageModification(sockets[id], msg, db_path, userDirPath, files, dirs);
                        }
                    }catch(std::exception& e){
                        update_n_threads(-1);
                        std::ostringstream error;
                        error << e.what();
                        Log_Writer.writeLog(error);
                        eraseSocket(sockets,id,socket_mutex);
                        std::lock_guard<std::mutex>lg(users_mutex);
                        for(auto it=users_connected.begin();it!=users_connected.end();++it){
                            if(it->second == id){ // find username by socket id
                                users_connected.erase(it->first);
                                break;
                            }
                        }
                    }
                });
                t.detach();
                id++;
                while(sockets.count(id) != 0)
                    id = (id + 1)%(2*MAX_THREADS); // id can assume values between 0 and 2*MAX_THREADS - 1 (no oevrflow)
            }
        }
        default: {
            std::cout<<"child: "<<pid<<std::endl;
        }
    }
    return 0;
}

void update_n_threads(const int value){
    std::lock_guard<std::mutex>lg(n_threads_mutex);
    n_threads += value;
}

int get_n_threads(){
    std::lock_guard<std::mutex>lg(n_threads_mutex);
    return n_threads;
}