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
                    //ROOT INITIALIZATION
                    //root_ptr->setName(userDirPath); // set the root as root_path/<username>
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
            sendFile(s, db_path, db_path);
        } else if (msg == "Database up to date") {
            //OK
        } else {
            // error
        }

        while (1) {
            msg = rcvMsg(s);
            const char *delimiter = " ";
            //char *token = std::strtok(const_cast<char *>(msg.c_str()), delimiter);
            //int i = 0;
            std::string path;
            std::string name;
            std::string type;
            std::string operation;
            /*while (token != NULL) {
                if (i == 0) type = std::string(token);
                if (i == 1) path = std::string(token);
                if (i == 2) operation = std::string(token);
                token = std::strtok(NULL, delimiter);
                i++;
            }*/
            // DIR marco marco created
            std::cout<<"msg: "<<msg<<std::endl;
            operation = msg.substr(msg.find_last_of(" ")+1); // created
            msg = msg.substr(0, msg.find_last_of(" ")); // DIR marco marco
            type = msg.substr(0,msg.find_first_of(" ")); // DIR
            msg = msg.substr(msg.find_first_of(" ")+1); // marco marco
            //path = msg.substr(msg.find_first_of(" ")+1, msg.find_last_of(" ")); // marco marco
            path = msg;

            std::cout<<"computed: path: "<<path<<" op: "<<operation<<" type: "<<type<<std::endl;

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
                    if(insertFileIntoDB(db_path, file))
                        std::cout<<"File inserito correttamente sul DB"<<std::endl;
                    else
                        std::cout<<"Problema nell'inserire il file sul DB"<<std::endl;
                } else if (operation == "erased") {
                    if (deleteFileFromDB(db_path, files[path]))
                        std::cout<<"File cancellato correttamente sul DB"<<std::endl;
                    else
                        std::cout<<"Problema nel cancellare il file sul DB"<<std::endl;
                    father.lock()->removeFile(name);
                    files.erase(path);
                    sendMsg(s, "DONE");
                } else if (operation == "modified") {
                    if (deleteFileFromDB(db_path, files[path]))
                        std::cout<<"File cancellato correttamente sul DB"<<std::endl;
                    else
                        std::cout<<"Problema nel cancellare il file sul DB"<<std::endl;
                    father.lock()->removeFile(name);
                    files.erase(path);
                    sendMsg(s, "READY");
                    rcvFile(s, userDirPath + "/" + path);
                    sendMsg(s, "DONE");
                    std::shared_ptr<File> file = father.lock()->addFile(name, computeDigest(userDirPath + "/"+path), false);
                    files[file->getPath()] = file;
                    if(insertFileIntoDB(db_path, file))
                        std::cout<<"File inserito correttamente sul DB"<<std::endl;
                    else
                        std::cout<<"Problema nell'inserire il file sul DB"<<std::endl;
                } else {
                    //errore
                    std::cout << "Stringa non ricevuta correttamente" << std::endl;
                    sendMsg(s, "ERROR");
                }
            } else if (type == "DIR") {
                //dirs modification handler

                if (operation == "created") {
                    std::cout << "\t sto per creare una cartella\n";
                    std::shared_ptr<Directory> dir = father.lock()->addDirectory(name, true);
                    dirs[dir->getPath()] = dir;
                    if (insertDirectoryIntoDB(db_path, dir))
                        std::cout<<"Directory inserita correttamente sul DB"<<std::endl;
                    else
                        std::cout<<"Problema nell'inserire la directory sul DB"<<std::endl;
                    sendMsg(s, "DONE");
                } else if (operation == "erased") {
                    if(deleteDirectoryFromDB(db_path, dirs[path]))
                        std::cout<<"Directory cancellata correttamente sul DB"<<std::endl;
                    else
                        std::cout<<"Problema nel cancellare la directory sul DB"<<std::endl;

                    father.lock()->removeDir(name);
                    dirs.erase(path);
                    sendMsg(s, "DONE");
                } else {
                    //errore
                    std::cout << "Stringa non ricevuta correttamente" << std::endl;
                    sendMsg(s, "ERROR");
                }
            } else {
                std::cout << "sono in else" << std::endl;
                //error
                //sendMsg(s, "ERROR");
                return -1;
            }
            //stampaFilesEDirs(files, dirs);
        }
    }
}
