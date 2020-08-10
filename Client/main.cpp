/*
 * Client
 * 
 */

#include <iostream>
#include <map>
#include <mutex>
#include <atomic>
#include <vector>
#include <thread>


// database mySql libraries
#include <sqlite3.h>

#include "./DB/Database.h"


// Socket
#include "./TCP_Socket/Socket.h"

#include "./FileManager/Directory.h"
#include "./FileManager/File.h"
#include "./FileManager/FileWatcher.h"
#include "./Crypto/MyCryptoLibrary.cpp"

#define PORT 5071

Socket s;

auto modification_function = [](const std::string file, FileStatus fs, FileType ft){
    std::string FT,FS,res;


    std::cout<<file;
    if(ft == FileType::file)
        std::cout<<" file";
    else
        std::cout<<" directory";
    switch (fs) {
        case FileStatus::created:
            std::cout<<" created\n";
            break;
        case FileStatus::erased:
            std::cout<<" erased\n";
            break;
        case FileStatus::modified:
            std::cout<<" modified\n";
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
    s.sendMsg(FT+" "+file+" "+FS); // FILE ./xx/yyy/zz.txt created
    res = s.rcvMsg();
    if(res.find("ERROR") == 0){ // example message: "ERROR <error info>"
        //error routine
    }
    if(res == "DONE" && ft == FileType::directory) // directory created/modified successfully on server. Job completed
        return;
    if(res == "DONE" && fs == FileStatus::erased) // file or directory cancellation needs only a "DONE" message
        return;
    if(res == "READY" && ft == FileType::file){
        s.sendFile(file);
        res = s.rcvMsg();
        if(res == "DONE") // file sended correctly
            return;
    }
    // if all went good, the code is already returned
    // error routine

};

void checkDB(const std::string& userDB_name, const std::string& serverDB_name,  std::unordered_map<std::string,File>& files, std::unordered_map<std::string,Directory>& dirs){
    Database userDB(userDB_name);
    Database serverDB(serverDB_name);
    std::unordered_map<std::string,FileStatus> fs_files; //<path,FileStatus>
    std::unordered_map<std::string,FileStatus> fs_dirs;  //<path,FileStatus>
    std::vector<File> userFiles, serverFiles;
    std::vector<Directory> userDirs, serverDirs;
    int nUserFiles, nUserDirs, nServerFiles, nServerDirs;

    userDB.DB_open();
    serverDB.DB_open();

    // user queries
    //userDB.DB_query("SELECT * FROM File",nUserFiles,userFiles.data());
    //userDB.DB_query("SELECT * FROM Directory",nUserDirs,userDirs.data());
    // server queries
    serverDB.DB_query("SELECT * FROM File",nServerFiles,serverFiles.data());
    serverDB.DB_query("SELECT * FROM Directory",nServerDirs,serverDirs.data());



    // populate maps
    for(auto it = files.begin(); it != files.end(); ++it){
        fs_files.insert(std::pair<std::string,FileStatus>(it->second.getPath(),FileStatus::none));
    }
    for(auto it = dirs.begin(); it != dirs.end(); ++it){
        fs_dirs.insert(std::pair<std::string,FileStatus>(it->second.getPath(),FileStatus::none));
    }

    // find what is different
    // in directories
    for(auto it = serverDirs.begin(); it != serverDirs.end(); ++it){
        if(fs_dirs.count(it->getPath()) == 0) // directory not present in user, it has been erased
            modification_function(it->getPath(),FileStatus::erased,FileType::directory); // send modification to server
    }
    for(auto it = fs_dirs.begin(); it != fs_dirs.end(); ++it){
        if(it->second == FileStatus::none){ // take a look at the comment above
            modification_function(it->first,FileStatus::created,FileType::directory);
        }
    }

    // in files
    for(auto it = serverFiles.begin(); it != serverFiles.end(); ++it){
        if(fs_files.count(it->getPath()) == 0){ // file not present in user, it has been erased
            modification_function(it->getPath(),FileStatus::erased,FileType::file); // send modification to server
        }else{
            if(!compareDigests(it->getHash(),files[it->getPath()].getHash())){ // hash are different: it has been updated on client
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


int main(int argc, char** argv)
{
    FileWatcher fw("./TestPath/",std::chrono::milliseconds(5000));
    std::map<std::string, File> files; // <path,File>
    std::map<std::string, Directory> dirs; // <path, Directory>

    //TODO: files and dir inizialize function

    s.inizialize_and_connect(PORT,AF_INET,"127.0.0.1");


    // SYN with server completed, starting to monitor client directory
    fw.start(modification_function);

    /*
    Database DB("../DB/user.db");
    Directory records[10];
    File files[10];
    int n_rec,n_files;
    DB.DB_open();
    DB.DB_query("SELECT * FROM DIRECTORY",n_rec,records);
    DB.DB_query("SELECT * FROM FILE",n_files,files);
    DB.DB_close();

    printf("\n");
    std::cout<<n_rec<<"\n";
    for(int i=0;i<n_rec;i++){
        std::cout<<"directory["<<i<<"]-> id: "<<records[i].getId()<<" path: "<<records[i].getPath()<<std::endl;
    }
    for(int i=0;i<n_files;i++){
        std::cout<<"files["<<i<<"]-> id: "<<files[i].getId()<<" id_dir: "<<files[i].getIdDir()<<" nome: "<<files[i].getName()<<" hash: "<<files[i].getHash()<<std::endl;
    }

    Socket s{};
    struct sockaddr_in addr;
    unsigned int len = sizeof(addr);


    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr)<=0){
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    s.connect(&addr, len);

    s.syncRequest("ciao");
    s.compareDBDigest("./DB/ciao.txt");

    s.sendDir("./client_directory/prova");
    std::cout<<s.rcvMsg()<<std::endl;

    s.sendFile("./client_directory/file.txt");
    std::cout<<s.rcvMsg()<<std::endl;
    */

    return 0;
}

