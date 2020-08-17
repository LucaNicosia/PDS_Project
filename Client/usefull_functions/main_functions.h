//
// Created by root on 14/08/20.
//

#ifndef PDS_PROJECT_CLIENT_MAIN_FUNCTIONS_H
#define PDS_PROJECT_CLIENT_MAIN_FUNCTIONS_H

#include <iostream>
#include "../FileManager/File.h"
#include "../FileManager/Directory.h"
#include "../Communication/Communication.h"
#include "../FileManager/FileWatcher.h"
#include "../DB/Database.h"
#include <map>

void stampaFilesEDirs(std::map<std::string,std::shared_ptr<File>> files, std::map<std::string,std::shared_ptr<Directory>> dirs){
    int i=0;
    for(auto it = files.begin(); it != files.end(); ++it,i++){
        std::cout<<"cont: "<<i<<" "<<it->second->getPath()<<"\n";
        std::cout<<"cont: "<<i<<" "<<it->second->getFatherPath()<<"\n";
        std::cout<<"cont: "<<i<<" "<<it->second->getHash()<<"\n";
        //std::cout<<"file: "<<it->second->getName()<<" "<<it->second->getPath()<<" "<<it->second->getHash()<<"\n";
    }
    i=0;
    for(auto it = dirs.begin(); it != dirs.end(); ++it,i++){
        std::cout<<"cont: "<<i<<" ";
        std::cout<<it->second->toString()<<"\n";
    }
}


void checkDB(const std::string& userDB_name, const std::string& serverDB_name,  std::map<std::string,std::shared_ptr<File>>& files, std::map<std::string,std::shared_ptr<Directory>>& dirs, const std::function<void (std::string, FileStatus, FileType)> &modification_function){
    //Database userDB(userDB_name);
    Database serverDB(serverDB_name);
    std::unordered_map<std::string,FileStatus> fs_files; //<path,FileStatus>
    std::unordered_map<std::string,FileStatus> fs_dirs;  //<path,FileStatus>
    std::vector<File> userFiles, serverFiles;
    std::vector<Directory> userDirs, serverDirs;
    int nUserFiles, nUserDirs, nServerFiles, nServerDirs;

    //userDB.DB_open();
    serverDB.open();

    // user queries
    //userDB.DB_query("SELECT * FROM File",nUserFiles,userFiles.data());
    //userDB.DB_query("SELECT * FROM Directory",nUserDirs,userDirs.data());
    // server queries
    serverDB.select("SELECT * FROM File",nServerFiles,serverFiles);
    serverDB.select("SELECT * FROM Directory",nServerDirs,serverDirs);

    for(int i=0;i<nServerFiles;i++){
        std::cout<<serverFiles[i].getPath()<<"\n";
    }
    for(int i=0;i<nServerDirs;i++){
        std::cout<<serverDirs[i].getPath()<<"\n";
    }



    // populate maps
    for(auto it = files.begin(); it != files.end(); ++it){
        fs_files.insert(std::pair<std::string,FileStatus>(it->second->getPath(),FileStatus::none));
    }
    for(auto it = dirs.begin(); it != dirs.end(); ++it){
        fs_dirs.insert(std::pair<std::string,FileStatus>(it->second->getPath(),FileStatus::none));
    }

    // find what is different
    // in directories
    for(auto it = serverDirs.begin(); it != serverDirs.end(); ++it){
        if(fs_dirs.count(it->getPath()) == 0) // directory not present in user, it has been erased
            modification_function(it->getPath(),FileStatus::erased,FileType::directory); // send modification to server
        else
            fs_dirs[it->getPath()] = FileStatus::created; // if it is already in 'fs_dirs' it has been already present

    }
    for(auto it = fs_dirs.begin(); it != fs_dirs.end(); ++it){
        if(it->second == FileStatus::none){ // 'none' records are those directories that where present on client but not on the DB
            modification_function(it->first,FileStatus::created,FileType::directory);
        }
    }

    // in files
    for(auto it = serverFiles.begin(); it != serverFiles.end(); ++it){
        if(fs_files.count(it->getPath()) == 0){ // file not present in user, it has been erased
            modification_function(it->getPath(),FileStatus::erased,FileType::file); // send modification to server
        }else{
            if(!compareDigests(it->getHash(),files[it->getPath()]->getHash())){ // hash are different: it has been updated on client
                modification_function(it->getPath(),FileStatus::modified,FileType::file);
                fs_files[it->getPath()] = FileStatus::modified; // if at the end of the loop, there are still some 'fs_files' with state 'none', it means that they are new
            } else {
                fs_files[it->getPath()] = FileStatus::created;
            }
        }
    }
    for(auto it = fs_files.begin(); it != fs_files.end(); ++it){
        if(it->second == FileStatus::none){ // take a look at the comment above
            modification_function(it->first,FileStatus::created,FileType::file);
        }
    }
}

void initialize_files_and_dirs(std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, const std::string& path, std::string& root, const std::string& path_to_db){
    // if path is "./xxx/" will become "./xxx"
    std::ifstream db_file(path_to_db,std::ios::in);
    Database db(path_to_db);
    bool is_db_new = false;
    if(!db_file){
        is_db_new = true;
        // user database doesn't exits, create it
        std::cout<<"Database doesn't exists"<<std::endl;

        db.open();
        db.exec("CREATE TABLE \"DIRECTORY\" ("
                "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT, "
                "\"path\" TEXT NOT NULL UNIQUE"
                ")");
        db.exec("CREATE TABLE \"FILE\" ("
                "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT, "
                "\"path\" TEXT NOT NULL UNIQUE, "
                "\"hash\" TEXT NOT NULL"
                ")");
    } else {
        // database already exists
        std::cout<<"Database alredy exists"<<std::endl;
        db_file.close();
    }
    if(path.find_last_of("/") == path.size()-1){
        root = path.substr(0,path.find_last_of("/"));
    }else{
        root = path;
    }

    for(auto &it : std::filesystem::recursive_directory_iterator(path)) {
        std::cout<<"\telem: "<<it.path().string()<<std::endl;
        if(it.is_directory()){
            std::weak_ptr<Directory> father = std::weak_ptr<Directory>();
            if(it.path().has_parent_path() && it.path().parent_path() != root){
                father = dirs[it.path().parent_path()]->getSelf();
            }
            dirs[it.path().string()] = Directory::makeDirectory(it.path().string(),father);
            if(is_db_new){
                db.exec("INSERT INTO DIRECTORY(path)\n"
                        "VALUES (\""+it.path().string()+"\")");
            }
        } else {
            std::weak_ptr<Directory> file_father;
            if(it.path().has_parent_path() && it.path().parent_path() != root){
                file_father = dirs[it.path().parent_path().string()]->getSelf();
            }
            files[it.path().string()] = std::make_shared<File>(it.path().string(),computeDigest(it.path().string()),file_father);
            if(is_db_new){
                db.exec("INSERT INTO FILE (path, hash)\n"
                        "VALUES (\""+it.path().string()+"\",\""+computeDigest(it.path().string())+"\")");
            }
        }
    }
}

int updateDB(const std::string& db_path, std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs){
    Database db(db_path);
    std::ifstream db_file(db_path);
    std::vector<File> db_Files;
    std::vector<Directory> db_Dirs;
    int n_record;

    if(!db_file){
        // a file that doesn't exits
        return -1;
    }
    db.open();

    db.select("SELECT * FROM FILE",n_record,db_Files);
    db.select("SELECT * FROM DIRECTORY",n_record,db_Dirs);

    for(auto it = files.begin(); it != files.end(); ++it){
        bool found = false;
        auto it2 = db_Files.begin();
        for(it2; it2 != db_Files.end(); ++it2){
            if(it2->getPath() == it->second->getPath()) {
                found = true;
                break;
            }
        }
        if(!found){
            db.exec("INSERT INTO FILE (path,hash) VALUES (\""+it->second->getPath()+"\",\""+it->second->getHash()+"\")");
2;      } else {
            if(!compareDigests(it2->getHash(),it->second->getHash())){
                db.exec("UPDATE FILE SET hash = \""+it->second->getHash()+"\" WHERE path = \""+it->second->getPath()+"\"");
            }
            db_Files.erase(it2); // delete from the vector
        }
    }
    if(!db_Files.empty()){
        for(auto it=db_Files.begin();it!=db_Files.end();++it){
            db.exec("DELETE FROM FILE WHERE path = \""+it->getPath()+"\"");
        }
    }
    for(auto it = dirs.begin(); it != dirs.end(); ++it){
        bool found = false;
        auto it2 = db_Dirs.begin();
        for(it2; it2 != db_Dirs.end(); ++it2) {
            if(it2->getPath() == it->second->getPath()) {
                found = true;
                break;
            }
        }
        if(!found){
            db.exec("INSERT INTO DIRECTORY (path) VALUES (\""+it->second->getPath()+"\")");
        } else {
            db_Dirs.erase(it2);
        }
    }
    if(!db_Dirs.empty()){
        for(auto it=db_Dirs.begin();it!=db_Dirs.end();++it){
            db.exec("DELETE FROM DIRECTORY WHERE path = \""+it->getPath()+"\"");
        }
    }

    db.close();
    return 0;
}

#endif //PDS_PROJECT_CLIENT_MAIN_FUNCTIONS_H
