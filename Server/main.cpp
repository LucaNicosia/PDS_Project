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

#define PORT 5100

ServerSocket ss(PORT);
std::map<std::string, Socket> sockets;

int function (char* name) {
// SYNC 'client'
    std::shared_ptr<Directory> root;
    std::string db_path;
    std::map<std::string, std::shared_ptr<File>> files; // <path,File>
    std::map<std::string, std::shared_ptr<Directory>> dirs; // <path, Directory>
    std::string username, userDirPath;
    std::string root_path = "server_directory";
    int cont = 0;
    while (true) {
        try {
            if (rcvSyncRequest(sockets[name], username, root_path, root, files, dirs) != 0) {
                std::cout << "Errore in SYNC\n";
                throw 20;
            } else {
                db_path = "../DB/" + username + ".db";
                userDirPath = root_path + "/" + username;
                break;
            }

        } catch (...) {
            //TODO: da rifare la catch
            if (username == "") {
                // invalid username -> throw
                throw std::runtime_error("error during sync: invalid username");
            }
            if (++cont == 3) exit(-1);
            db_path = "../DB/" + username + ".db";
            userDirPath = root_path + "/" + username;
            check_user_data(userDirPath, db_path); // try to create db and directory
            //ROOT INITIALIZATION
            //root_ptr->setName(userDirPath); // set the root as root_path/<username>
        }
    }
    std::string msg;
    msg = rcvMsg(sockets[name]);
    if (msg == "GET-DB") { // client asks for server.db database version
        if (sendFile(sockets[name], db_path, cleanPath(db_path, "../DB")) < 0) {
            // error handling
        }
    } else if (msg == "Database up to date") {
        //OK
    } else {
        // error
        std::cout << "error in main\n";
    }

    while (1) {
        msg = rcvMsg(sockets[name]);
        std::string path;
        std::string nameFD;
        std::string type;
        std::string operation;
        // get data from message 'msg'
        operation = msg.substr(msg.find_last_of(" ") + 1);
        msg = msg.substr(0, msg.find_last_of(" "));
        type = msg.substr(0, msg.find_first_of(" "));
        msg = msg.substr(msg.find_first_of(" ") + 1);
        path = msg;
        nameFD = path.substr(path.find_last_of("/") + 1);

        std::weak_ptr<Directory> father = dirs[Directory::getFatherFromPath(path)];
        if (type == "FILE") {
            // file modification handler
            if (operation == "created") {
                sendMsg(sockets[name], "READY");
                rcvFile(sockets[name], userDirPath + "/" + path);
                sendMsg(sockets[name], "DONE");
                std::shared_ptr<File> file = father.lock()->addFile(nameFD,
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
                father.lock()->removeFile(nameFD);
                files.erase(path);
                sendMsg(sockets[name], "DONE");
            } else if (operation == "modified") {
                if (!deleteFileFromDB(db_path, files[path])) {
                    std::cout << "Problema nel cancellare il file sul DB" << std::endl;
                }
                father.lock()->removeFile(nameFD);
                files.erase(path);
                sendMsg(sockets[name], "READY");
                rcvFile(sockets[name], userDirPath + "/" + path);
                sendMsg(sockets[name], "DONE");
                std::shared_ptr<File> file = father.lock()->addFile(nameFD,
                                                                    computeDigest(userDirPath + "/" + path),
                                                                    false);
                files[file->getPath()] = file;
                if (!insertFileIntoDB(db_path, file)) {
                    std::cout << "Problema nell'inserire il file sul DB" << std::endl;
                }
            } else {
                //errore
                std::cout << "Stringa non ricevuta correttamente" << std::endl;
                sendMsg(sockets[name], "ERROR");
            }
        } else if (type == "DIR") {
            //dirs modification handler
            if (operation == "created") {
                std::shared_ptr<Directory> dir = father.lock()->addDirectory(nameFD, true);
                dirs[dir->getPath()] = dir;
                if (!insertDirectoryIntoDB(db_path, dir)) {
                    std::cout << "Problema nell'inserire la directory sul DB" << std::endl;
                }
                sendMsg(sockets[name], "DONE");
            } else if (operation == "erased") {
                if (!deleteDirectoryFromDB(db_path, dirs[path])) {
                    std::cout << "Problema nel cancellare la directory sul DB" << std::endl;
                }
                father.lock()->removeDir(nameFD);
                dirs.erase(path);
                sendMsg(sockets[name], "DONE");
            } else {
                //errore
                std::cout << "Stringa non ricevuta correttamente" << std::endl;
                sendMsg(sockets[name], "ERROR");
            }
        } else {
            std::cout << "unknown message type" << std::endl;
            //error
            //sendMsg(s, "ERROR");
            return -1;
        }

    }
}


int main() {
    File f;
    File f2 {};
    Directory d;

    while (true) {

        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        std::cout << "Waiting for incoming connections at port " << PORT << "..." << std::endl;
        Socket s = ss.accept(&addr, len);


        char name[16];
        if (inet_ntop(AF_INET, &addr.sin_addr, name, sizeof(name)) == nullptr)
            throw std::runtime_error("Cannot convert");
        std::cout << "Got a connection from " << name << ":" << ntohs(addr.sin_port) << "\n";

        sockets[name] = std::move(s);

        std::thread t ([&name](){
            function(name);
        });

        t.detach();

        }
}
