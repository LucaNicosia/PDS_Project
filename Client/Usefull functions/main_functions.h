#ifndef MAIN_FUNCTIONS_H
#define MAIN_FUNCTIONS_H

#define DEBUG 0

#include <iostream>
#include "../Entities/File/File.h"
#include "../Entities/Directory/Directory.h"
#include "Communication/Communication.h"
#include "../Entities/FileWatcher/FileWatcher.h"
#include "../Entities/Database/Database.h"
#include "../Entities/Exceptions/MyExceptions.h"
#include <map>
#include <queue>
#include "utilities.h"
#include "constants.h"


namespace fs = std::filesystem;

std::string cleanPath(const std::string & path, const std::string& rubbish);
void printFilesAndDirs(std::map<std::string, std::shared_ptr<File>> files, std::map<std::string, std::shared_ptr<Directory>> dirs);
int insertFileIntoDB(const std::string& db_path, std::shared_ptr<File>& file);
int deleteFileFromDB(const std::string& db_path, const std::shared_ptr<File>& file);
int updateFileDB(const std::string& db_path, std::shared_ptr<File>& file);
int deleteDB(const std::string& db_path, const std::shared_ptr<Directory>& dir);
int insertDirectoryIntoDB(const std::string& db_path, std::shared_ptr<Directory>& dir);
void checkDB(const std::string& dir_path, const std::string& userDB_name, const std::string& serverDB_name,  std::map<std::string,std::shared_ptr<File>>& files, std::map<std::string,std::shared_ptr<Directory>>& dirs, const std::function<void (std::string, std::string, FileStatus, FileType)> &modification_function, std::shared_ptr<Directory>& root);
void initialize_files_and_dirs(std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::string& path, const std::string& path_to_db, std::shared_ptr<Directory>& root);
int updateDB(const std::string& db_path, std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::shared_ptr<Directory>& root);
std::string compute_db_digest(std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs);
void manageModification(Socket& s, std::string msg,const std::string& db_path, const std::string& userDirPath ,std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs);
void restore(Socket& s, std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::string& path, const std::string& db_path, bool dir_is_empty, std::shared_ptr<Directory>& root);
void changeRunningState(bool& var, const bool value, std::mutex& m);
bool checkRunnigState(bool& var, std::mutex& m);

std::string cleanPath(const std::string & path, const std::string& rubbish){ // es. "TestPath/ciao.txt" -> "ciao.txt"
    std::string head = path;
    std::string tail;
    while(rubbish != head){
        tail = path.substr(head.find_last_of("/")+1);
        head = head.substr(0, head.find_last_of("/"));
    }
    return tail;
}

void printFilesAndDirst(std::map<std::string, std::shared_ptr<File>> files, std::map<std::string, std::shared_ptr<Directory>> dirs){
    std::cout<<"***DIRECTORIES***"<<std::endl;
    for (const auto& x : dirs) {
        std::cout << x.first << ": " << x.second->toString()<< std::endl;
    }
    std::cout<<"***FILES***"<<std::endl;
    for (const auto& x : files) {
        std::cout << x.first << ": " << x.second->toString()<< std::endl;
    }
}

int insertFileIntoDB(const std::string& db_path, std::shared_ptr<File>& file){
    Database db(db_path);
    db.open();
    std::vector<File> v;
    int n;
    db.select("SELECT * FROM FILE WHERE path = \""+file->getPath()+"\"",n,v);
    if(n == 0) { // New element
        db.exec("INSERT INTO FILE (path,hash,name) VALUES (\"" + file->getPath() + "\",\"" + file->getHash() + "\",\"" +file->getName() + "\")");
        return 0;
    }
    else if(!compareDigests(v[0].getHash(),file->getHash())) { // if present, update hash only
        updateFileDB(db_path,file);
        return 1;
    }
    db.close();
    return 0;
}

int deleteFileFromDB(const std::string& db_path, const std::shared_ptr<File>& file){
    Database db(db_path);
    db.open();
    db.exec("DELETE FROM FILE WHERE path = \""+file->getPath()+"\"");
    db.close();
    return 0;
}

int updateFileDB(const std::string& db_path, std::shared_ptr<File>& file){
    Database db(db_path);
    db.open();
    db.exec("UPDATE FILE SET hash = \""+file->getHash()+"\" WHERE path = \""+file->getPath()+"\"");
    db.close();
    return 0;
}

int insertDirectoryIntoDB(const std::string& db_path, std::shared_ptr<Directory>& dir){
    Database db(db_path);
    db.open();
    std::vector<Directory> v;
    int n;
    db.select("SELECT * FROM DIRECTORY WHERE path = \""+dir->getPath()+"\"",n,v);
    if(n == 0) {
        db.exec("INSERT INTO DIRECTORY (path,name) VALUES (\"" + dir->getPath() + "\", " + "\"" + dir->getName() +"\")");
        return 0;
    }
    db.close();
    return 1;
}
int deleteDirectoryFromDB(const std::string& db_path, const std::shared_ptr<Directory>& dir){
    Database db(db_path);
    db.open();
    db.exec("DELETE FROM DIRECTORY WHERE path = \""+dir->getPath()+"\"");
    db.close();
    for (int i = 0; i < dir->getDSons().size(); i++) {
        deleteDirectoryFromDB(db_path, dir->getDSons()[i]);
    }
    for (int i = 0; i < dir->getFSons().size(); i++) {
        deleteFileFromDB(db_path, dir->getFSons()[i]);
    }
    return 0;
}

void checkDB(const std::string& dir_path, const std::string& userDB_name, const std::string& serverDB_name,  std::map<std::string,std::shared_ptr<File>>& files, std::map<std::string,std::shared_ptr<Directory>>& dirs, const std::function<void (std::string, std::string, FileStatus, FileType)> &modification_function, std::shared_ptr<Directory>& root){
    // Database userDB(userDB_name);
    Database serverDB(serverDB_name);
    std::map<std::string,FileStatus> fs_files; //<path,FileStatus>
    std::map<std::string,FileStatus> fs_dirs;  //<path,FileStatus>
    std::vector<File> userFiles, serverFiles;
    std::vector<Directory> userDirs, serverDirs;
    int nServerFiles, nServerDirs;
    std::vector<struct action_data> dirs_er;

    serverDB.open();

    serverDB.select("SELECT * FROM FILE",nServerFiles,serverFiles);
    serverDB.select("SELECT * FROM DIRECTORY",nServerDirs,serverDirs);

    // Populate maps
    for(auto it = files.begin(); it != files.end(); ++it){
        fs_files.insert(std::pair<std::string,FileStatus>(it->second->getPath(),FileStatus::none));
    }
    for(auto it = dirs.begin(); it != dirs.end(); ++it){
        if(it->second->getPath() == root->getPath())
            continue; // root is not added to fs_dirs and DB
        fs_dirs.insert(std::pair<std::string,FileStatus>(it->second->getPath(),FileStatus::none));
    }

    // Find what is different
    // in directories
    for(auto it = serverDirs.begin(); it != serverDirs.end(); ++it){
        if(fs_dirs.count(it->getPath()) == 0) { // Directory not present in user, it has been erased
            struct action_data x;
            x.filePath = dir_path + "/" + it->getPath();
            x.file = it->getName();
            x.fs = FileStatus::erased;
            x.ft = FileType::directory;
            dirs_er.push_back(x);
        }
        else
            fs_dirs[it->getPath()] = FileStatus::created; // If it is already in 'fs_dirs' it has been already created

    }
    for(auto it = fs_dirs.begin(); it != fs_dirs.end(); ++it){
        if(it->second == FileStatus::none){ // 'none' records are those directories that where present on client but not on the DB
            modification_function(it->first.substr(it->first.find_last_of("/")+1),dir_path+"/"+it->first,FileStatus::created,FileType::directory);
        }
    }

    // in files
    for(auto it = serverFiles.begin(); it != serverFiles.end(); ++it){
        if(fs_files.count(it->getPath()) == 0){ // File not present in user, it has been erased
            modification_function(it->getName(),dir_path+"/"+it->getPath(),FileStatus::erased,FileType::file); // send modification to server
        }else{
            if(!compareDigests(it->getHash(),files[it->getPath()]->getHash())){ // hash are different: it has been updated on client
                modification_function(it->getName(),dir_path+"/"+it->getPath(),FileStatus::modified,FileType::file);
                fs_files[it->getPath()] = FileStatus::modified; // If at the end of the loop, there are still some 'fs_files' with state 'none', it means that they are new
            } else {
                fs_files[it->getPath()] = FileStatus::created;
            }
        }
    }
    for(auto it = fs_files.begin(); it != fs_files.end(); ++it){
        if(it->second == FileStatus::none){ // Take a look at the comment above
            modification_function(it->first.substr(it->first.find_last_of("/")+1),dir_path+"/"+it->first,FileStatus::created,FileType::file);
        }
    }

    // After i deleted all the files, delete also directories
    for(int i=dirs_er.size()-1; i>=0; i--){
        struct action_data x = dirs_er[i];
        modification_function(x.file,x.filePath,x.fs,x.ft);
    }
}

void initialize_files_and_dirs(std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::string& path, const std::string& path_to_db, std::shared_ptr<Directory>& root){
    // If path is "./xxx/" will become "./xxx"
    std::ifstream db_file(path_to_db,std::ios::in);
    Database db(path_to_db);
    bool is_db_new = false;
    if(!db_file){
        is_db_new = true;
        // User database doesn't exits, create it
        if (DEBUG)
            std::cout<<"Database doesn't exists"<<std::endl;
        std::ofstream myDB(path_to_db); // create file
        myDB.close();
        db.open();
        db.exec("CREATE TABLE \"DIRECTORY\" ("
                "\"path\" TEXT PRIMARY KEY NOT NULL UNIQUE,"
                "\"name\" TEXT NOT NULL"
                ")");
        db.exec("CREATE TABLE \"FILE\" ("
                "\"path\" TEXT PRIMARY KEY NOT NULL UNIQUE, "
                "\"name\" TEXT NOT NULL,"
                "\"hash\" TEXT NOT NULL"
                ")");
        db.close();
    } else {
        // Database already exists
        if (DEBUG)
        std::cout<<"Database already exists"<<std::endl;
        db_file.close();
    }
    dirs[""] = root; //setting 'root' in dirs[""]

    for(auto &it : std::filesystem::recursive_directory_iterator(path)) {
        std::string this_path = cleanPath(it.path().string(),path);
        std::string father_path = (cleanPath(it.path().parent_path().string(),path) == path)?"":cleanPath(it.path().parent_path().string(),path); // if the father of this element is "root" set it as ""
        if(it.is_directory()){
            std::weak_ptr<Directory> father = std::weak_ptr<Directory>();
            if(it.path().has_parent_path()){
                father = dirs[father_path]->getSelf();
                dirs[this_path] = father.lock()->addDirectory(it.path().filename().string(),false);
            } else {
                // Error handling
                throw general_exception("cannot access to parent path");
            }
            if(is_db_new){
                insertDirectoryIntoDB(path_to_db,dirs[this_path]);
            }
        } else {
            std::weak_ptr<Directory> dir_father;
            if(it.path().has_parent_path()){
                dir_father = dirs[father_path]->getSelf();
                files[this_path] = dir_father.lock()->addFile(it.path().filename().string(),computeDigest(it.path().string()),false);
            }
            else{
                // Error handling
                throw general_exception("cannot access to parent path");
            }
            if(is_db_new){
                insertFileIntoDB(path_to_db,files[this_path]);
            }
        }
    }
}

int updateDB(const std::string& db_path, std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::shared_ptr<Directory>& root){
    Database db(db_path);
    std::vector<File> db_Files;
    std::vector<Directory> db_Dirs;
    int n_record;

    db.open();
    db.select("SELECT * FROM FILE",n_record,db_Files);
    db.select("SELECT * FROM DIRECTORY",n_record,db_Dirs);
    db.close();

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
            insertFileIntoDB(db_path,it->second);
2;      } else {
            if(!compareDigests(it2->getHash(),it->second->getHash())){
                updateFileDB(db_path,it->second);
            }
            db_Files.erase(it2); // delete from the vector
        }
    }
    if(!db_Files.empty()){
        for(auto it=db_Files.begin();it!=db_Files.end();++it){
            deleteFileFromDB(db_path,std::make_shared<File>(*it));
        }
    }
    for(auto it = dirs.begin(); it != dirs.end(); ++it){
        if(it->second->getPath() == dirs[root->getPath()]->getPath()){
            // root is not added to DB
            continue;
        }
        bool found = false;
        auto it2 = db_Dirs.begin();
        for(it2; it2 != db_Dirs.end(); ++it2) {
            if(it2->getPath() == it->second->getPath()) {
                found = true;
                break;
            }
        }
        if(!found){
            insertDirectoryIntoDB(db_path,it->second);
        } else {
            db_Dirs.erase(it2);
        }
    }
    if(!db_Dirs.empty()){
        for(auto it=db_Dirs.begin();it!=db_Dirs.end();++it){
            deleteDirectoryFromDB(db_path,std::make_shared<Directory>(*it));
        }
    }

    return 0;
}

std::string connectRequest(Socket& s, const std::string username, const std::string password, const std::string mode){
    sendMsg(s, std::string ("CONNECT "+username+" "+password+" "+mode));
    std::string msg = rcvMsg(s); // msg = CONNECT-OK
    if (msg == "CONNECT-ERROR"){
        //CONNECT-ERROR
        return std::string("CONNECT-ERROR");
    }else{
        //CONNECT-OK
        sendMsg(s,"CONNECT-OK");
        std::string digest;
        while((digest = rcvMsg(s)) == "updating database"); // While he receive "updating database" he keeps waiting
        if(digest.find("DIGEST") == 0) // ok
            return std::string (digest.substr(digest.find(" ")+1));
        else
            return digest; // Digest contains error message
    }
}

// Server and client may have different order on db, compute the digest manually
std::string compute_db_digest(std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs){
    // Files and dirs contains data of db and also stored in memory
    for(auto it : files){
        appendDigest(it.second->toString().c_str(),it.second->toString().length());
    }
    for(auto it : dirs){
        if(it.second->getPath() == "") // Root is not included in db
            continue;
        appendDigest(it.second->toString().c_str(),it.second->toString().length());
    }
    return getAppendedDigest();
}

void manageModification(Socket& s, std::string msg,const std::string& db_path, const std::string& userDirPath ,std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs){
    std::string path;
    std::string name;
    std::string type;
    std::string operation;
    // Get data from message 'msg'
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
            sendMsg(s, "READY");
            std::string digest = rcvFile(s, userDirPath + "/" + path);
            sendMsg(s, "DONE");
            std::shared_ptr<File> file = father.lock()->addFile(name,
                                                                digest,
                                                                false);
            files[file->getPath()] = file;
            insertFileIntoDB(db_path, file);
        } else if (operation == "erased") {
            deleteFileFromDB(db_path, files[path]);
            father.lock()->removeFile(name);
            files.erase(path);
            sendMsg(s, "DONE");
        } else if (operation == "modified") {
            deleteFileFromDB(db_path, files[path]);
            father.lock()->removeFile(name);
            files.erase(path);
            sendMsg(s, "READY");
            std::string digest = rcvFile(s, userDirPath + "/" + path);
            sendMsg(s, "DONE");
            std::shared_ptr<File> file = father.lock()->addFile(name,
                                                                digest,
                                                                false);
            files[file->getPath()] = file;
            insertFileIntoDB(db_path, file);
        } else {
            // Error
            sendMsg(s, "ERROR");
            throw general_exception("unknown-message");
        }
    } else if (type == "DIR") {
        // Dirs modification handler
        if (operation == "created") {
            std::shared_ptr<Directory> dir = father.lock()->addDirectory(name, true);
            dirs[dir->getPath()] = dir;
            insertDirectoryIntoDB(db_path, dir);
            sendMsg(s, "DONE");
        } else if (operation == "erased") {
            deleteDirectoryFromDB(db_path, dirs[path]);
            father.lock()->removeDir(name);
            dirs.erase(path);
            sendMsg(s, "DONE");
        } else if (operation == "modified") {
            // Nothing to do
        } else {
            // Error
            sendMsg(s, "ERROR");
            throw general_exception("unknown-message");
        }
    } else {
        if (DEBUG)
            std::cout << "unknown message type" << std::endl;
        throw std::runtime_error("unknown message type");
    }
}


void restore(Socket& s, std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::string& path, const std::string& db_path, bool dir_is_empty, std::shared_ptr<Directory>& root){
    sendMsg(s,"restore");
    if(!dir_is_empty){ // Delete content before restoring
        // Remove all files and directories
        for(auto &it : std::filesystem::directory_iterator(path)){
            fs::remove_all(it.path());
        }
        // Reset maps
        files.clear();
        dirs.clear();
        // Set again root in 'dirs'
        dirs[""] = root;
        // Reset database
        Database db(db_path);
        db.open();
        db.exec("DELETE FROM FILE");
        db.exec("DELETE FROM DIRECTORY");
        db.close();
        // Initialize files and dirs (empty) and create again the database
        initialize_files_and_dirs(files,dirs,path,db_path,root);
    }
    while(true){
        std::string msg = rcvMsg(s);
        if(msg == "restore completed"){
            return;
        }
        manageModification(s,msg,db_path,path,files,dirs);
    }
}

void changeRunningState(bool& var, const bool value, std::mutex& m){
    std::lock_guard<std::mutex> lg(m);
    var = value;
}

bool checkRunnigState(bool& var, std::mutex& m){
    std::lock_guard<std::mutex> lg(m);
    return var;
}

#endif //MAIN_FUNCTIONS_H
