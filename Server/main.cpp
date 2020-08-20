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

#define PORT 5104
#define MAXFD 50000

ServerSocket ss(PORT);
std::map<std::string, std::shared_ptr<File>> files; // <path,File>
std::map<std::string, std::shared_ptr<Directory>> dirs; // <path, Directory>
std::string db_path;

void stampaFilesEDirs(void){
    std::cout<<"***DIRECTORIES***"<<std::endl;
    for (const auto& x : dirs) {
        std::cout << x.first << ": " << x.second->toString()<< std::endl;
    }

    std::cout<<"***FILES***"<<std::endl;
    for (const auto& x : files) {
        std::cout << x.first << ": " << x.second->toString()<< std::endl;
    }
}

void initialize_files_and_dirs(/*std::map<std::string, std::shared_ptr<File>> files, std::map<std::string, std::shared_ptr<Directory>> dirs, */std::string path, std::string db_path){

    Database db(db_path);
    int nFiles, nDirs;
    std::vector<File> queryFiles;
    std::vector<Directory> queryDirs;

    Directory::setRoot(path);
    std::shared_ptr<Directory> root = Directory::getRoot();
    dirs[root->getPath()] = root;

    db.open();

    db.select("SELECT * FROM File",nFiles,queryFiles);
    db.select("SELECT * FROM Directory",nDirs,queryDirs);


    for (int i = 0; i < nDirs; i++){
        std::weak_ptr<Directory> father;
        father = dirs[Directory::getFatherFromPath(queryDirs[i].getPath())]->getSelf();
        std::shared_ptr<Directory> dir = father.lock()->addDirectory(queryDirs[i].getName(), false);
        dirs[dir->getPath()] = dir;
    }

    for (int i = 0; i < nFiles; i++){
        std::size_t found = queryFiles[i].getPath().find_last_of("/");
        queryFiles[i].setName(queryFiles[i].getPath().substr(found+1));
        std::weak_ptr<Directory> father = dirs[Directory::getFatherFromPath(queryFiles[i].getPath())]->getSelf();
        std::shared_ptr<File> file = father.lock()->addFile(queryFiles[i].getName(), queryFiles[i].getHash(), false);
        files[file->getPath()] = file;
    }

    stampaFilesEDirs();
    root->ls(4);
}

int main() {
    File f;
    File f2 {};
    Directory d;
    pthread_t threads[100];
    std::string path = "TestPath";

    while (true){

        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        std::cout<<"Waiting for incoming connections at port "<<PORT<<"..."<<std::endl;
        Socket s = ss.accept(&addr, len);
        std::string username;

        char name[16];
        if (inet_ntop(AF_INET, &addr.sin_addr, name, sizeof(name)) == nullptr) throw std::runtime_error("Cannot convert");
        std::cout<<"Got a connection from "<<name<<":"<<ntohs(addr.sin_port)<<"\n";

        // SYNC 'client'
        if(rcvSyncRequest(s,username) != 0){
            std::cout<<"Errore\n";
        }else{
            db_path = "../DB/"+username+".db";
        }
        std::string msg;
        msg = rcvMsg(s);
        if(msg == "GET-DB"){ // client asks for server.db database version
            sendFile(s,db_path);
        } else if(msg == "Database up to date"){
            //OK
        } else {
            // error
        }

        //Populate files and dirs
        initialize_files_and_dirs(/*files, dirs, */path, db_path);

        while(1) {
            msg = rcvMsg(s);
            const char* delimiter = " ";
            char* token = std::strtok(const_cast<char*>(msg.c_str()), delimiter);
            int i = 0;
            std::string path;
            std::string name;
            std::string type;
            std::string operation;
            while (token != NULL){
                if (i == 0) type = std::string(token);
                if (i == 1) path = std::string(token);
                if (i == 2) operation = std::string(token);
                token = std::strtok(NULL, delimiter);
                i++;
            }

            name = path.substr(path.find("/")+1, path.size());
            std::weak_ptr<Directory> father = dirs[Directory::getFatherFromPath(path)];
            if(type == "FILE"){
                // file modification handler

                if (operation == "created"){

                    std::shared_ptr<File> file = father.lock()->addFile(name, computeDigest(path), false);
                    files[file->getPath()] = file;
                    sendMsg(s, "READY");
                    rcvFile(s, file->getPath());
                    sendMsg(s, "DONE");
                }else if (operation == "erased"){
                    //TODO: non funziona
                    father.lock()->removeFile(name);
                    files.erase(path);
                    sendMsg(s, "DONE");
                }else if (operation == "modified"){
                    father.lock()->removeFile(name);
                    files.erase(path);
                    sendMsg(s, "READY");
                    std::shared_ptr<File> file = father.lock()->addFile(name, computeDigest(path), false);
                    files[file->getPath()] = file;
                    rcvFile(s, file->getPath());
                    sendMsg(s, "DONE");
                }else{
                    //errore
                    std::cout<<"Stringa non ricevuta correttamente"<<std::endl;
                    sendMsg(s, "ERROR");
                }
            } else if(type == "DIR"){
                //dirs modification handler

                if (operation == "created"){
                    std::shared_ptr<Directory> dir = father.lock()->addDirectory(name, true);
                    dirs[dir->getPath()] = dir;
                    sendMsg(s,"DONE");
                }else if (operation == "erased"){
                    father.lock()->removeDir(name);
                    dirs.erase(path);
                    sendMsg(s,"DONE");
                }else{
                    //errore
                    std::cout<<"Stringa non ricevuta correttamente"<<std::endl;
                    sendMsg(s, "ERROR");
                }
            } else {
                std::cout<<"sono in else"<<std::endl;
                //error
                //sendMsg(s, "ERROR");
                //return -1;
            }

        }


        //TEST DIR 'path'
        //std::cout<<"Stringa ricevuta dal client: "<<s.rcvDir()<<std::endl;
        //s.sendMsg("OK");

        //TEST FILE 'path'
        //s.rcvFile("./server_directory/file.txt");
    }

    /*f.setDigest(computeDigest("prova.txt"));
    f2.setDigest(computeDigest("prova2.txt"));

    compareDigests(f.getDigest(), f.getDigest());
    compareDigests(f.getDigest(), f2.getDigest());*/


}
