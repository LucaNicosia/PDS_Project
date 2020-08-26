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
#include "DB/Database.h"

// DB
#include <sqlite3.h>

#include <filesystem>

#include "usefull_functions/main_functions.h"

#define PORT 5108

ServerSocket ss(PORT);
std::shared_ptr<Directory> root;
std::map<std::string, std::shared_ptr<File>> files; // <path,File>
std::map<std::string, std::shared_ptr<Directory>> dirs; // <path, Directory>
std::string db_path;



int main() {
    File f;
    File f2 {};
    Directory d;
    pthread_t threads[100];

    while (true) {

        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        std::cout << "Waiting for incoming connections at port " << PORT << "..." << std::endl;
        Socket s = ss.accept(&addr, len);
        std::string username, root_path = "server_directory", userDirPath;


        char name[16];
        if (inet_ntop(AF_INET, &addr.sin_addr, name, sizeof(name)) == nullptr)
            throw std::runtime_error("Cannot convert");
        std::cout << "Got a connection from " << name << ":" << ntohs(addr.sin_port) << "\n";

        // SYNC 'client'
        int cont = 0;
        while(true) {
            try{
                if (rcvSyncRequest(s,username,root_path,root,files,dirs) != 0) {
                    std::cout << "Errore in SYNC\n";
                    throw 20;
                } else {
                    db_path = "../DB/" + username + ".db";
                    userDirPath = root_path + "/" + username;
                    break;
                }

            } catch (...) {
                //TODO: da rifare la catch
                if(username == ""){
                    // invalid username -> throw
                    throw std::runtime_error("error during sync: invalid username");
                }
                if(++cont == 3) exit(-1);
                db_path = "../DB/" + username + ".db";
                userDirPath = root_path + "/" + username;
                check_user_data(userDirPath, db_path); // try to create db and directory
                //ROOT INITIALIZATION
                //root_ptr->setName(userDirPath); // set the root as root_path/<username>
            }
        }
        std::string msg;
        msg = rcvMsg(s);
        if (msg == "GET-DB") { // client asks for server.db database version
            sendFile(s, db_path, cleanPath(db_path,"../DB"));
        } else if (msg == "Database up to date") {
            //OK
        } else {
            // error
            std::cout<<"error in main\n";
        }

        while (1) {
            msg = rcvMsg(s);
            std::string path;
            std::string name;
            std::string type;
            std::string operation;
            // get data from message 'msg'
            operation = msg.substr(msg.find_last_of(" ")+1);
            msg = msg.substr(0, msg.find_last_of(" "));
            type = msg.substr(0,msg.find_first_of(" "));
            msg = msg.substr(msg.find_first_of(" ")+1);
            path = msg;
            name = path.substr(path.find_last_of("/") + 1);

            std::weak_ptr<Directory> father = dirs[Directory::getFatherFromPath(path)];
            if (type == "FILE") {
                // file modification handler
                if (operation == "created") {
                    sendMsg(s, "READY");
                    rcvFile(s, userDirPath+"/"+path);
                    sendMsg(s, "DONE");
                    std::shared_ptr<File> file = father.lock()->addFile(name, computeDigest(userDirPath + "/"+path), false);
                    files[file->getPath()] = file;
                    if(!insertFileIntoDB(db_path, file)) {
                        std::cout << "Problema nell'inserire il file sul DB" << std::endl;
                    }
                } else if (operation == "erased") {
                    if (!deleteFileFromDB(db_path, files[path])) {
                        std::cout << "Problema nel cancellare il file sul DB" << std::endl;
                    }
                    father.lock()->removeFile(name);
                    files.erase(path);
                    sendMsg(s, "DONE");
                } else if (operation == "modified") {
                    if (!deleteFileFromDB(db_path, files[path])) {
                        std::cout << "Problema nel cancellare il file sul DB" << std::endl;
                    }
                    father.lock()->removeFile(name);
                    files.erase(path);
                    sendMsg(s, "READY");
                    rcvFile(s, userDirPath + "/" + path);
                    sendMsg(s, "DONE");
                    std::shared_ptr<File> file = father.lock()->addFile(name, computeDigest(userDirPath + "/"+path), false);
                    files[file->getPath()] = file;
                    if(!insertFileIntoDB(db_path, file)) {
                        std::cout << "Problema nell'inserire il file sul DB" << std::endl;
                    }
                } else {
                    //errore
                    std::cout << "Stringa non ricevuta correttamente" << std::endl;
                    sendMsg(s, "ERROR");
                }
            } else if (type == "DIR") {
                //dirs modification handler
                if (operation == "created") {
                    std::shared_ptr<Directory> dir = father.lock()->addDirectory(name, true);
                    dirs[dir->getPath()] = dir;
                    if (!insertDirectoryIntoDB(db_path, dir)) {
                        std::cout << "Problema nell'inserire la directory sul DB" << std::endl;
                    }
                    sendMsg(s, "DONE");
                } else if (operation == "erased") {
                    if(!deleteDirectoryFromDB(db_path, dirs[path])) {
                        std::cout << "Problema nel cancellare la directory sul DB" << std::endl;
                    }
                    father.lock()->removeDir(name);
                    dirs.erase(path);
                    sendMsg(s, "DONE");
                } else {
                    //errore
                    std::cout << "Stringa non ricevuta correttamente" << std::endl;
                    sendMsg(s, "ERROR");
                }
            } else {
                std::cout << "unknown message type" << std::endl;
                //error
                //sendMsg(s, "ERROR");
                return -1;
            }
        }
    }
}
