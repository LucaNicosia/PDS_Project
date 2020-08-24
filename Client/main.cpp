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

#define PORT 5108
#define MAXFD 50000


Socket s;
Directory root;
std::shared_ptr<Directory> root_ptr;
std::map<std::string, std::shared_ptr<File>> files; // <path,File>
std::map<std::string, std::shared_ptr<Directory>> dirs; // <path, Directory>
std::string db_path = "../DB/user.db";
bool synchronized = false;
std::string path = "TestPath";


auto modification_function = [](const std::string file, const std::string filePath, FileStatus fs, FileType ft){ // file is the file name
    std::string FT,FS,res;
    std::string cleaned_path = cleanPath(filePath,path);
    std::cout<<file;
    if(ft == FileType::file)
        std::cout<<" file";
    else
        std::cout<<" directory";
    std::weak_ptr<Directory> father;
    switch (fs) {
        case FileStatus::created:
            std::cout << " created\n";
            father = dirs[Directory::getFatherFromPath(cleaned_path)]->getSelf();
            if (ft == FileType::directory) {
                dirs[cleaned_path] = father.lock()->addDirectory(file, false);
                if (synchronized)
                if (insertDirectoryIntoDB(db_path, dirs[cleaned_path]))
                    std::cout<<"Directory inserita correttamente sul DB"<<std::endl;
                else
                    std::cout<<"Problema nell'inserire la directory sul DB"<<std::endl;
            } else {// file
                files[cleaned_path] = std::make_shared<File>(file,computeDigest(filePath), father);
                if (synchronized)
                if(insertFileIntoDB(db_path, files[cleaned_path]))
                    std::cout<<"File inserito correttamente sul DB"<<std::endl;
                else
                    std::cout<<"Problema nell'inserire il file sul DB"<<std::endl;
            }
            break;
        case FileStatus::erased:
            std::cout<<" erased\n";
            if (ft == FileType::directory) {
                if (synchronized)
                if(deleteDirectoryFromDB(db_path, dirs[cleaned_path]))
                    std::cout<<"Directory cancellata correttamente sul DB"<<std::endl;
                else
                    std::cout<<"Problema nel cancellare la directory sul DB"<<std::endl;
                dirs.erase(cleaned_path);
            } else { // file
                if (synchronized)
                if (deleteFileFromDB(db_path, files[cleaned_path]))
                    std::cout<<"File cancellato correttamente sul DB"<<std::endl;
                else
                    std::cout<<"Problema nel cancellare il file sul DB"<<std::endl;
                files.erase(cleaned_path);
            }
            break;
        case FileStatus::modified:
            std::cout<<" modified\n";
            // file only: directories are modified when its content is modified. No action needed
            if(ft == FileType::file){
                files[cleaned_path]->setHash(computeDigest(filePath));
                if (synchronized)
                if(updateFileDB(db_path, files[cleaned_path]))
                    std::cout<<"File aggiornato correttamente sul DB"<<std::endl;
                else
                    std::cout<<"Problema nell'aggiornare il file sul DB"<<std::endl;
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
    sendMsg(s,FT+" "+cleaned_path+" "+FS); // FILE ./xx/yyy/zz.txt created
    res = rcvMsg(s);
    if(res.find("ERROR") == 0){ // example message: "ERROR <error info>"
        //error routine
    }
    if(res == "DONE" && ft == FileType::directory) // directory created/modified successfully on server. Job completed
        return;
    if(res == "DONE" && fs == FileStatus::erased) // file or directory cancellation needs only a "DONE" message
        return;
    if(res == "READY" && ft == FileType::file){
        sendFile(s,filePath,cleaned_path);
        res = rcvMsg(s);
        if(res == "DONE") // file sended correctly
            return;
    }
    std::cout<<"modification function ended"<<std::endl;
    // if all went good, the code is already returned
    // error routine
    stampaFilesEDirs(files, dirs);
};


int main(int argc, char** argv)
{

    //ROOT INITIALIZATION
    root_ptr = root.makeDirectory(path, std::weak_ptr<Directory>());

    std::string username = "user";
    FileWatcher fw(path,std::chrono::milliseconds(5000));
    // inizialization of data structures
    initialize_files_and_dirs(files, dirs, path, db_path, root_ptr);
    updateDB(db_path,files,dirs, root_ptr);
    stampaFilesEDirs(files,dirs);
    // connect to the remote server
    s.setTimeoutSecs(20);
    s.setTimeoutUsecs(0);
    s.inizialize_and_connect(PORT,AF_INET,"127.0.0.1");
    // sync with the server
    int cont = 0;
    std::string server_digest;

    while(true) {
        try {
            server_digest = syncRequest(s, username);
            if(server_digest == "SYNC-ERROR")
                throw 20;
            break;
        } catch (int p) {
            if (++cont == 3) {
                std::cout << "stop! some error\n";
                exit(-1);
            }
        }
    }
    // check if the DB is updated
    if(!compareDigests(server_digest,b64_encode(computeDigest("../DB/"+username+".db")))){
        std::cout<<"server DB is not updated\n";
        // get DB from server
        sendMsg(s,"GET-DB");
        rcvFile(s,"../DB/server.db");
        // check which files and directories aren't updated
        checkDB(path,"","../DB/server.db",files,dirs,modification_function, root_ptr);
        std::cout<<"--- checkDB ended ---\n";
    } else {
        std::cout<<"server DB is updated\n";
        sendMsg(s,"Database up to date");
    }
    // SYN with server completed, starting to monitor client directory
    synchronized = true;
    std::thread t1([&fw]() { fw.start(modification_function); });
    std::cout<<"--- System ready ---\n";
    //std::this_thread::sleep_for(std::chrono::seconds(5000));
    //fw.stop();
    t1.join();
    /**/
    return 0;
}

