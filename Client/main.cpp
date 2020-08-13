/*
 * Client
 * 
 */

#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <atomic>
#include <vector>
#include <thread>
#include <filesystem>

// database mySql libraries
#include <sqlite3.h>

#include "./DB/Database.h"

// Communication
#include "Communication/Communication.h"

// Socket
//#include "./TCP_Socket/Socket.h"

#include "./FileManager/Directory.h"
#include "./FileManager/File.h"
#include "./FileManager/FileWatcher.h"

#define PORT 5074
#define MAXFD 50000

Socket s;
std::map<std::string, std::shared_ptr<File>> files; // <path,File>
std::map<std::string, std::shared_ptr<Directory>> dirs; // <path, Directory>
std::string root;

void stampaFilesEDirs(){
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

auto modification_function = [](const std::string file, FileStatus fs, FileType ft){
    std::string FT,FS,res;

    std::cout<<file;
    if(ft == FileType::file)
        std::cout<<" file";
    else
        std::cout<<" directory";
    std::weak_ptr<Directory> father;
    switch (fs) {
        case FileStatus::created:
            std::cout << " created\n";
            if(Directory::getFatherFromPath(file) != root){
                father = dirs[Directory::getFatherFromPath(file)]->getSelf();
            }
            if (ft == FileType::directory) {
                dirs[file] = Directory::makeDirectory(0,file,father);
            } else {// file
                files[file] = std::make_shared<File>(file, 0, 0, computeDigest(file), father);
            }
            break;
        case FileStatus::erased:
            std::cout<<" erased\n";
            if (ft == FileType::directory) {
                dirs.erase(file);
            } else { // file
                files.erase(file);
            }
            break;
        case FileStatus::modified:
            std::cout<<" modified\n";
            // file only: directories are modified when its content is modified. No action needed
            if(ft == FileType::file){
                files[file]->setHash(computeDigest(file));
            }
            break;
    }

    switch (ft) {
        case FileType::file:
            FT = "FILE";
            break;
        case FileType::directory:
            FT = "DIR";
            break;
        default:
            std::cout<<"unknown FileType\n"; // exception
    }
    switch(fs){
        case FileStatus::created:
            FS = "created";
            break;
        case FileStatus::erased:
            FS = "erased";
            break;
        case FileStatus::modified:
            FS = "modified";
            break;
        default:
            std::cout<<"unknown FileStatus\n"; // exception
    }
    sendMsg(s,FT+" "+file+" "+FS); // FILE ./xx/yyy/zz.txt created
    res = rcvMsg(s);
    if(res.find("ERROR") == 0){ // example message: "ERROR <error info>"
        //error routine
    }
    if(res == "DONE" && ft == FileType::directory) // directory created/modified successfully on server. Job completed
        return;
    if(res == "DONE" && fs == FileStatus::erased) // file or directory cancellation needs only a "DONE" message
        return;
    if(res == "READY" && ft == FileType::file){
        sendFile(s,file);
        res = rcvMsg(s);
        if(res == "DONE") // file sended correctly
            return;
    }
    // if all went good, the code is already returned
    // error routine
    //stampaFilesEDirs();
};

void checkDB(const std::string& userDB_name, const std::string& serverDB_name,  std::map<std::string,std::shared_ptr<File>>& files, std::map<std::string,std::shared_ptr<Directory>>& dirs){
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
            }
        }
    }
    for(auto it = fs_files.begin(); it != fs_files.end(); ++it){
        if(it->second == FileStatus::none){ // take a look at the comment above
            modification_function(it->first,FileStatus::created,FileType::file);
        }
    }
}

void initialize_files_and_dirs(std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, const std::string& path){
    // if path is "./xxx/" will become "./xxx"
    if(path.find_last_of("/") == path.size()-1){
        root = path.substr(0,path.find_last_of("/"));
    }else{
        root = path;
    }

    for(auto &it : std::filesystem::recursive_directory_iterator(path)) {
        if(it.is_directory()){
            std::weak_ptr<Directory> father = std::weak_ptr<Directory>();
            if(it.path().has_parent_path() && it.path().parent_path() != root){
                father = dirs[it.path().parent_path()]->getSelf();
            }
            dirs[it.path().string()] = Directory::makeDirectory(0,it.path().string(),father); // id FORSE inutile;
        } else {
            std::weak_ptr<Directory> file_father;
            if(it.path().has_parent_path() && it.path().parent_path() != root){
                file_father = dirs[it.path().parent_path().string()]->getSelf();
            }
            files[it.path().string()] = std::make_shared<File>(it.path().string(),0,0,computeDigest(it.path().string()),file_father);
        }
    }
}

int main(int argc, char** argv)
{
    std::string path = "./TestPath/";
    FileWatcher fw(path,std::chrono::milliseconds(5000));

    initialize_files_and_dirs(files,dirs,path);

    FileWatcher FW("./TestPath/",std::chrono::milliseconds(5000));

    s.inizialize_and_connect(PORT,AF_INET,"127.0.0.1");

    checkDB("","../DB/user.db",files,dirs);

    std::cout<<"--- checkDB ended ---\n";

    // SYN with server completed, starting to monitor client directory
    fw.start(modification_function);
    /**/
    return 0;
}

