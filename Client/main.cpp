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
#include "usefull_functions/main_functions.h"

#define PORT 5074
#define MAXFD 50000

Socket s;
std::map<std::string, std::shared_ptr<File>> files; // <path,File>
std::map<std::string, std::shared_ptr<Directory>> dirs; // <path, Directory>
std::string root;

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


int main(int argc, char** argv)
{
    std::string username = "user";
    std::string path = "./TestPath/";
    FileWatcher fw(path,std::chrono::milliseconds(5000));
    // inizialization of data structures
    initialize_files_and_dirs(files,dirs,path,root);
    // connect to the remote server
    s.inizialize_and_connect(PORT,AF_INET,"127.0.0.1");
    // sync with the server
    std::string server_digest = syncRequest(s,username);
    // check if the DB is updated
    if(!compareDigests(server_digest,b64_encode(computeDigest("../DB/"+username+".db")))){
        std::cout<<"server DB is not updated\n";
        // get DB from server
        sendMsg(s,"GET-DB");
        rcvFile(s,"../DB/server.db");
        // check which files and directories aren't updated
        checkDB("","../DB/server.db",files,dirs,modification_function);
        std::cout<<"--- checkDB ended ---\n";
    } else {
        std::cout<<"server DB is updated\n";
        sendMsg(s,"Database up to date");
    }
    // SYN with server completed, starting to monitor client directory
    fw.start(modification_function);
    /**/
    return 0;
}

