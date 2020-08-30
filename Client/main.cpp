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

//#define PORT 5108
#define MAXFD 50000


Socket s;
Directory root;
std::shared_ptr<Directory> root_ptr;
std::map<std::string, std::shared_ptr<File>> files; // <path,File>
std::map<std::string, std::shared_ptr<Directory>> dirs; // <path, Directory>
std::string db_path = "../DB/user.db";
bool synchronized = false;
std::string path = "TestPath";
FileWatcher fw(path,std::chrono::milliseconds(1000));
std::string username = "user";
int port;
std::mutex action_server_mutex;

void connect_to_remote_server();
void action_on_server(std::string str);

auto modification_function = [](const std::string file, const std::string filePath, FileStatus fs, FileType ft){ // file is the file name
    std::string FT,FS,res;
    std::string cleaned_path = cleanPath(filePath,path);
    std::weak_ptr<Directory> father;

    action_on_server("mod func"); // before anything, check connection with server

    if(ft == FileType::directory && fs == FileStatus::modified) // in this case, nothing to do: dir modification means a creation or cancellation of a sub directory
        return;

    switch (fs) {
        case FileStatus::created:
            father = dirs[Directory::getFatherFromPath(cleaned_path)]->getSelf();
            if (ft == FileType::directory) {
                dirs[cleaned_path] = father.lock()->addDirectory(file, false);
                if (synchronized) {
                    if (!insertDirectoryIntoDB(db_path, dirs[cleaned_path]))
                        std::cout << "Problema nell'inserire la directory sul DB" << std::endl;
                }
            } else {// file
                files[cleaned_path] = std::make_shared<File>(file,computeDigest(filePath), father);
                if (synchronized) {
                    if (!insertFileIntoDB(db_path, files[cleaned_path]))
                        std::cout << "Problema nell'inserire il file sul DB" << std::endl;
                }
            }
            break;
        case FileStatus::erased:
            if (ft == FileType::directory) {
                if (synchronized) {
                    if (!deleteDirectoryFromDB(db_path, dirs[cleaned_path]))
                        std::cout << "Problema nel cancellare la directory sul DB" << std::endl;
                }
                dirs.erase(cleaned_path);
            } else { // file
                if (synchronized) {
                    if (!deleteFileFromDB(db_path, files[cleaned_path]))
                        std::cout << "Problema nel cancellare il file sul DB" << std::endl;
                }
                files.erase(cleaned_path);
            }
            break;
        case FileStatus::modified:
            // file only: directories are modified when its content is modified. No action needed
            if(ft == FileType::file){
                files[cleaned_path]->setHash(computeDigest(filePath));
                if (synchronized) {
                    if (!updateFileDB(db_path, files[cleaned_path]))
                        std::cout << "Problema nell'aggiornare il file sul DB" << std::endl;
                }
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
        int ret = sendFile(s,filePath,cleaned_path);
        if(ret < 0){
            // error handling
        }
        if(ret == 0) {
            res = rcvMsg(s);
            if (res != "DONE") { // file sended correctly
                //error handling
            }
        }
        return;
    }
    // if all went good, the code is already returned
    // error routine
};

int main(int argc, char** argv)
{
    port = std::atoi(argv[1]);

    std::cout<<"> Inserisci username: ";
    std::cin>>username;

    //ROOT INITIALIZATION
    root_ptr = root.makeDirectory(path, std::weak_ptr<Directory>());

    // inizialization of data structures
    initialize_files_and_dirs(files, dirs, path, db_path, root_ptr);
    updateDB(db_path,files,dirs, root_ptr);

    // connect to server
    connect_to_remote_server();

    // SYN with server completed, starting to monitor client directory
    synchronized = true;
    std::thread t1([]() { fw.start(modification_function);});
    std::cout<<"--- System ready ---\n";
    //std::this_thread::sleep_for(std::chrono::seconds(5000));
    //fw.stop();
    std::thread t2([](){
        while(true) {
            // every seconds check if there are some work to do or not
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if(!fw.isRunning()) return;
            action_on_server("on thread");
        }
    });
    t1.join();
    t2.join();
    /**/
    return 0;
}

void connect_to_remote_server(){
    // connect to the remote server
    s = Socket();
    s.setTimeoutSecs(20);
    s.setTimeoutUsecs(0);
    s.inizialize_and_connect(port,AF_INET,"127.0.0.1");
    // sync with the server
    int cont = 0;
    std::string server_digest;
    std::string client_digest = compute_db_digest(files,dirs);

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
    if(!compareDigests(server_digest,client_digest)){
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
        if(rcvMsg(s) != "server_db_ok"){
            std::cout<<"error in db response on 'server_db_ok'\n";
            exit(-1);
        }
    }
}

void action_on_server(std::string str){ // this function is used in a loop to check the state of the FileWatcher
    std::unique_lock<std::mutex>lg(action_server_mutex);
    if(!fw.isRunning()) return;
    FileWatcher_state last, cur;
    fw.getAllState(last,cur);

    if(!s.is_open() && cur == FileWatcher_state::mod_found && last == FileWatcher_state::ready){ // first modification found, open the socket
        lg.unlock();
        connect_to_remote_server(); // this function can call 'action_on_server', deadlock without unlock
        lg.lock();
    }
    if(s.is_open() && (cur == FileWatcher_state::ended || cur == FileWatcher_state::ready && last == FileWatcher_state::ended)){
        //std::cout<<">> action -> close\n";
        sendMsg(s,"update completed");
        s.close();
    }
}