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
#include <exception>

// database mySql libraries
#include <sqlite3.h>

#include "Entities/Database/Database.h"

// Communication
#include "Usefull functions/Communication/Communication.h"

// Socket
//#include "./TCP_Socket/Socket.h"

#include "Entities/Directory/Directory.h"
#include "Entities/File/File.h"
#include "Entities/FileWatcher/FileWatcher.h"
#include "Usefull functions/main_functions.h"
#include "Entities/Exceptions/MyExceptions.h"


//#define PORT 5108
#define MAXFD 50000
#define RESTORE 1
#define UPDATED 0
#define PATH_TO_DB "../DB"
#define INITIAL_PATH "TestPath"

Socket s;
std::shared_ptr<Directory> root_ptr;
std::map<std::string, std::shared_ptr<File>> files; // <path,File>
std::map<std::string, std::shared_ptr<Directory>> dirs; // <path, Directory>
std::string db_path = PATH_TO_DB;
std::string server_db_path = PATH_TO_DB;
bool synchronized = false;
std::string path = INITIAL_PATH;
FileWatcher fw;
std::string username;
std::string password;
std::string mode;
int port;
std::mutex action_server_mutex;

int connect_to_remote_server(bool needs_restore);
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
                    insertDirectoryIntoDB(db_path, dirs[cleaned_path]);
                }
            } else {// file
                files[cleaned_path] = std::make_shared<File>(file,computeDigest(filePath), father);
                if (synchronized) {
                    insertFileIntoDB(db_path, files[cleaned_path]);
                }
            }
            break;
        case FileStatus::erased:
            if (ft == FileType::directory) {
                if (synchronized) {
                    deleteDirectoryFromDB(db_path, dirs[cleaned_path]);
                }
                dirs.erase(cleaned_path);
            } else { // file
                if (synchronized) {
                    deleteFileFromDB(db_path, files[cleaned_path]);
                }
                files.erase(cleaned_path);
            }
            break;
        case FileStatus::modified:
            // file only: directories are modified when its content is modified. No action needed
            if(ft == FileType::file){
                files[cleaned_path]->setHash(computeDigest(filePath));
                if (synchronized) {
                    updateFileDB(db_path, files[cleaned_path]);
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
            throw general_exception("unknown FileType");
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
            throw general_exception("unknown FileStatus");
    }

    sendMsg(s,FT+" "+cleaned_path+" "+FS); // FILE ./xx/yyy/zz.txt created
    res = rcvMsg(s);
    if(res.find("ERROR") == 0){ // example message: "ERROR <error info>"
        //error routine
        throw general_exception("ERROR message: "+res);
    }
    if(res == "DONE" && ft == FileType::directory) // directory created/modified successfully on server. Job completed
        return;
    if(res == "DONE" && fs == FileStatus::erased) // file or directory cancellation needs only a "DONE" message
        return;
    if(res == "READY" && ft == FileType::file){
        int ret = sendFile(s,filePath,cleaned_path);
        if(ret == 0) {
            res = rcvMsg(s);
            if (res != "DONE") { // file sended correctly
                //error handling
                throw general_exception("unknown-message");
            }
        }
        return;
    }
    // if all went good, the code is already returned
    // error
    throw general_exception("code reached forbidden line");
};

int main(int argc, char** argv)
{
    pid_t pid = fork();
    switch(pid) {
        case -1: {
            throw std::runtime_error("cannot fork");
        }
        case 0: {
            int round_count = 0;
            while (round_count++ < 3) { // try 3 times to recover from a problem
                try {
                    if (argc != 5) {
                        throw std::runtime_error("not enough arguments - usage PORT USERNAME PASSWORD MODE");
                    }
                    port = std::atoi(argv[1]);
                    username = std::string(argv[2]);
                    password = std::string(argv[3]);
                    mode = std::string(argv[4]);
                    db_path += "/" + username + ".db";
                    server_db_path += "/" + username + "_server.db";
                    path += "/" + username;
                    if (!std::filesystem::is_directory(path)) {
                        std::filesystem::create_directory(path);
                    }

                    //ROOT INITIALIZATION
                    root_ptr = std::make_shared<Directory>()->makeDirectory(path, std::weak_ptr<Directory>());

                    // inizialization of data structures
                    initialize_files_and_dirs(files, dirs, path, db_path, root_ptr);
                    updateDB(db_path, files, dirs, root_ptr);

                    // connect to server
                    // if 'files' and 'dirs' are empty, no file stored -> restore needed
                    // 'dirs.empty() == false ' is always true because in 'dirs' is stored 'root'
                    int ret = connect_to_remote_server((files.empty() && (dirs.size() == 1)) || (mode == "RESTORE"));

                    if (ret == RESTORE) {
                        restore(s, files, dirs, path, db_path, files.empty() && (dirs.size() == 1), root_ptr);
                    }

                    // SYN with server completed, starting to monitor client directory
                    synchronized = true;
                    std::thread t1([]() {
                        fw.set(path, std::chrono::milliseconds(1000));
                        fw.start(modification_function);
                    });
                    std::cout << "--- System ready ---\n";
                    round_count = 0; // when the syncrosization is ended, reset the "try to connect" counter
                    std::thread t2([]() {
                        while (true) {
                            // every seconds check if there are some work to do or not
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                            if (!fw.isRunning()) return;
                            action_on_server("on thread");
                        }
                    });
                    t1.join();
                    t2.join();
                    return 0;
                } catch (socket_exception &se) {
                    // reset variable and retry
                    std::cout << "socket_exc: " << se.what() << std::endl;
                    s.close();
                    root_ptr = nullptr;
                    dirs.clear();
                    files.clear();
                    path = INITIAL_PATH;
                    db_path = PATH_TO_DB;
                    server_db_path = PATH_TO_DB;
                    synchronized = false;
                    std::this_thread::sleep_for(std::chrono::seconds(3)); // wait 3 seconds before reconnection
                } catch (filesystem_exception &fe) {
                    std::cout << "filesystem_exc: " << fe.what() << std::endl;
                    s.close();
                    dirs.clear();
                    files.clear();
                    return -1; // critical problem, redo the same thing doesn't resolve the problem
                } catch (database_exception &de) {
                    std::cout << "database_exc: " << de.what() << std::endl;
                    s.close();
                    dirs.clear();
                    files.clear();
                    return -1; // critical problem, redo the same thing doesn't resolve the problem
                } catch (general_exception &ge) {
                    std::cout << "general_exc: " << ge.what() << std::endl;
                    s.close();
                    dirs.clear();
                    files.clear();
                    return -1; // critical problem, redo the same thing doesn't resolve the problem
                } catch (std::exception &e) {
                    std::cout << "exc: " << e.what() << std::endl;
                    s.close();
                    dirs.clear();
                    files.clear();
                    return -1; // critical problem, redo the same thing doesn't resolve the problem
                }
            }
            return -1;
        }
        default: {
            std::cout<<"child pid: "<<pid<<std::endl;
            break;
        }
    }

    return 0;
}

int connect_to_remote_server(bool needs_restore){
    // connect to the remote server
    s = Socket();
    s.setTimeoutSecs(20);
    s.setTimeoutUsecs(0);
    try {
        s.inizialize_and_connect(port, AF_INET, "127.0.0.1");
    } catch(std::runtime_error& e){
        throw socket_exception(e.what());
    }
    // sync with the server
    int cont = 0;
    std::string server_digest;
    std::string client_digest = compute_db_digest(files,dirs);

    while(true) {
        try {
            server_digest = connectRequest(s, username, password, mode);
            std::cout<<server_digest<<std::endl;
            if(server_digest == "CONNECT-ERROR")
                throw general_exception("connect-error");
            break;
        } catch (general_exception& ge) {
            if (++cont == 3) {
                std::cout << "Wrong username and/or password!\n";
                exit(-1);
            }
        }
    }

    // CONNECT-OK
    if (!needs_restore){
        // check if the DB is updated
        if(!compareDigests(server_digest,client_digest)){
            std::cout<<"server DB is not updated\n";
            // get DB from server
            sendMsg(s,"GET-DB");
            rcvFile(s,server_db_path);
            // check which files and directories aren't updated
            checkDB(path,"",server_db_path,files,dirs,modification_function, root_ptr);
            std::cout<<"--- checkDB ended ---\n";
        } else {
            std::cout<<"server DB is updated\n";
            sendMsg(s,"Database up to date");
            if(rcvMsg(s) != "server_db_ok"){
                std::cout<<"error in db response on 'server_db_ok'\n";
                exit(-1);
            }
        }
    }else{ // restore
        if(compareDigests(server_digest,client_digest)){ // if digest are the same, also the server is empty: no actions
            sendMsg(s, "Database up to date");
        } else {
            // server has some data, needs to restore
            return RESTORE;
        }
    }
    return UPDATED;
}

void action_on_server(std::string str){ // this function is used in a loop to check the state of the FileWatcher
    std::unique_lock<std::mutex>lg(action_server_mutex);
    if(!fw.isRunning()) return;
    FileWatcher_state last, cur;
    fw.getAllState(last,cur);

    if(!s.is_open() && cur == FileWatcher_state::mod_found && last == FileWatcher_state::ready){ // first modification found, open the socket
        lg.unlock();
        connect_to_remote_server(false); // this function can call 'action_on_server', deadlock without unlock
        lg.lock();
    }
    if(s.is_open() && (cur == FileWatcher_state::ended || cur == FileWatcher_state::ready && last == FileWatcher_state::ended)){
        //std::cout<<">> action -> close\n";
        sendMsg(s,"update completed");
        s.close();
    }
}