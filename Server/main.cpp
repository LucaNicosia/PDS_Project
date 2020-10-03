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

            while(rcvConnectRequest(sockets[id], username, password, mode) != 0){
                std::cout<<"Wrong username and/or password\n"<<std::endl;
            }

            if (mode == "FETCH"){
                // old code for fetch mode
                std::cout<<"FETCH MODE"<<std::endl;
                while(rcvSyncRequest(sockets[id],username, root_path,root,files,dirs) != 0) {
                    std::cout << "Errore in SYNC\n";
                }
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
                    sendMsg(sockets[id],"server_db_ok");
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
                        std::string path;
                        std::string name;
                        std::string type;
                        std::string operation;
                        // get data from message 'msg'
                        operation = msg.substr(msg.find_last_of(" ") + 1);
                        msg = msg.substr(0, msg.find_last_of(" "));
                        type = msg.substr(0, msg.find_first_of(" "));
                        msg = msg.substr(msg.find_first_of(" ") + 1);
                        path = msg;
                        name = path.substr(path.find_last_of("/") + 1);

                        std::weak_ptr<Directory> father = dirs[Directory::getFatherFromPath(path)];
                        if (type == "FILE") {
                            // file modification handler
                            if (operation == "created") {
                                sendMsg(sockets[id], "READY");
                                rcvFile(sockets[id], userDirPath + "/" + path);
                                sendMsg(sockets[id], "DONE");
                                std::shared_ptr<File> file = father.lock()->addFile(name,
                                                                                    computeDigest(userDirPath + "/" + path),
                                                                                    false);
                                files[file->getPath()] = file;
                                if (!insertFileIntoDB(db_path, file)) {
                                    std::cout << "Problema nell'inserire il file sul DB" << std::endl;
                                }
                            } else if (operation == "erased") {
                                if (!deleteFileFromDB(db_path, files[path])) {
                                    std::cout << "Problema nel cancellare il file sul DB" << std::endl;
                                }
                                father.lock()->removeFile(name);
                                files.erase(path);
                                sendMsg(sockets[id], "DONE");
                            } else if (operation == "modified") {
                                if (!deleteFileFromDB(db_path, files[path])) {
                                    std::cout << "Problema nel cancellare il file sul DB" << std::endl;
                                }
                                father.lock()->removeFile(name);
                                files.erase(path);
                                sendMsg(sockets[id], "READY");
                                rcvFile(sockets[id], userDirPath + "/" + path);
                                sendMsg(sockets[id], "DONE");
                                std::shared_ptr<File> file = father.lock()->addFile(name,
                                                                                    computeDigest(userDirPath + "/" + path),
                                                                                    false);
                                files[file->getPath()] = file;
                                if (!insertFileIntoDB(db_path, file)) {
                                    std::cout << "Problema nell'inserire il file sul DB" << std::endl;
                                }
                            } else {
                                //errore
                                std::cout << "Stringa non ricevuta correttamente (" << type << " " << path << " "
                                          << operation << ")" << std::endl;
                                sendMsg(sockets[id], "ERROR");
                            }
                        } else if (type == "DIR") {
                            //dirs modification handler
                            if (operation == "created") {
                                std::shared_ptr<Directory> dir = father.lock()->addDirectory(name, true);
                                dirs[dir->getPath()] = dir;
                                if (!insertDirectoryIntoDB(db_path, dir)) {
                                    std::cout << "Problema nell'inserire la directory sul DB" << std::endl;
                                }
                                sendMsg(sockets[id], "DONE");
                            } else if (operation == "erased") {
                                if (!deleteDirectoryFromDB(db_path, dirs[path])) {
                                    std::cout << "Problema nel cancellare la directory sul DB" << std::endl;
                                }
                                std::cout<<"fuori da erased"<<std::endl;
                                //std::cout<<"father: "<<dirs[Directory::getFatherFromPath(path)]->toString()<<std::endl; // TODO: questo non funziona in hard start
                                father.lock()->removeDir(name);
                                std::cout<<"dopo removeDir"<<std::endl;
                                dirs.erase(path);
                                std::cout<<"dopo dirs.erase"<<std::endl;
                                sendMsg(sockets[id], "DONE");
                            } else if (operation == "modified") {
                                // nothing to do
                            } else {
                                //errore
                                std::cout << "Stringa non ricevuta correttamente (" << type << " " << path << " "
                                          << operation << ")" << std::endl;
                                sendMsg(sockets[id], "ERROR");
                            }
                        } else {
                            std::cout << "unknown message type" << std::endl;
                            //error
                            //sendMsg(sockets[id], "ERROR");
                            //return;
                            //goto restart;
                            throw std::runtime_error("unknown message type");
                        }
                    }
                } catch (std::exception& e) {
                    std::cout<<"sono in catch\n";
                    std::cout<<e.what()<<std::endl;
                    //sockets.erase(id);
                    eraseSocket(sockets,id);
                    return;
                }
            }else if (mode == "RESTORE"){
                //TODO: insert your code for restore mode
                std::cout<<"RESTORE MODE"<<std::endl;
            }else{
                std::cout<<"UNKNOWN MODE "<<mode<<std::endl;
            }


        });
        t.detach();
        id++; // increment 'id' for next socket in vector
    }
}
