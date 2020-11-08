#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <atomic>
#include <vector>
#include <thread>
#include <filesystem>
#include <exception>
#include <signal.h>

#include "Usefull functions/Communication/Communication.h"
#include "Entities/Directory/Directory.h"
#include "Entities/File/File.h"
#include "Entities/FileWatcher/FileWatcher.h"
#include "Usefull functions/main_functions.h"
#include "Entities/Exceptions/MyExceptions.h"
#include "Usefull functions/constants.h"

#define MAXFD 50000
#define RESTORE 1
#define UPDATED 0

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

int connect_to_remote_server(bool needs_restore, int* p);
void action_on_server();
void resetVariables(const std::string exc_type, const std::exception& e, std::mutex& m, bool& all_threads_running);

auto modification_function = [](const std::string file, const std::string filePath, FileStatus fs, FileType ft){ // file is the file name
    std::string FT,FS,res;
    std::string cleaned_path = cleanPath(filePath,path);
    std::weak_ptr<Directory> father;

    action_on_server(); // Before anything, check connection with server

    if(ft == FileType::directory && fs == FileStatus::modified) // In this case, nothing to do: dir modification means a creation or cancellation of a sub directory
        return;

    switch (fs) {
        case FileStatus::created:
            father = dirs[Directory::getFatherFromPath(cleaned_path)]->getSelf();
            if (ft == FileType::directory) {
                dirs[cleaned_path] = father.lock()->addDirectory(file, false);
                if (synchronized) {
                    insertDirectoryIntoDB(db_path, dirs[cleaned_path]);
                }
            } else { // File
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
            // File only: directories are modified when its content is modified. No action needed
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
    if(res == "DONE" && ft == FileType::directory) // Directory created/modified successfully on server. Job completed
        return;
    if(res == "DONE" && fs == FileStatus::erased) // File or directory cancellation needs only a "DONE" message
        return;
    if(res == "READY" && ft == FileType::file){
        int ret = sendFile(s,filePath,cleaned_path);
        if(ret == 0) {
            res = rcvMsg(s);
            if (res != "DONE") { // File sent correctly
                // Error handling
                throw general_exception("unknown-message");
            }
        }
        return;
    }
    // If all went good, the code is already returned
    // Error
    throw general_exception("code reached forbidden line");
};

int main(int argc, char** argv)
{
    if (argc != 5) {
        std::string error("not enough arguments - usage PORT USERNAME PASSWORD MODE");
        throw std::runtime_error(error);
    }
    std::mutex thread_checker;
    bool all_threads_running = true;
    int p[2];
    if(pipe(p) < 0)
        throw std::runtime_error("cannot create pipe");
    pid_t pid = fork();
    switch(pid) {
        case -1: {
            throw std::runtime_error("cannot fork");
        }
        case 0: {
            close(p[0]);
            if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) // Ignore SIGPIPE to check if pipe is broken
                throw std::runtime_error("cannot ignore SIGPIPE");
            Log_Writer.setLogFilePath(LOG_PATH);
            Log_Writer.setUseMutex(true);
            int round_count = 0;
            while (round_count++ < 3) { // Try 3 times to recover from a problem
                try
                {
                    changeRunningState(all_threads_running,true,thread_checker);
                    port = std::atoi(argv[1]);
                    username = std::string(argv[2]);
                    password = std::string(argv[3]);
                    mode = std::string(argv[4]);
                    db_path =  std::string(PATH_TO_DB) + "/" + username + ".db";
                    server_db_path = std::string(PATH_TO_DB) + "/" + username + "_server.db";
                    path = std::string(INITIAL_PATH) + "/" + username;
                    if (!std::filesystem::is_directory(path)) {
                        std::filesystem::create_directory(path);
                    }

                    // ROOT INITIALIZATION
                    root_ptr = std::make_shared<Directory>()->makeDirectory(path, std::weak_ptr<Directory>());
                    // Inizialization of data structures
                    initialize_files_and_dirs(files, dirs, path, db_path, root_ptr);
                    updateDB(db_path, files, dirs, root_ptr);

                    // Connect to server
                    // if 'files' and 'dirs' are empty, no file stored -> restore needed
                    // 'dirs.empty() == false ' is always true because in 'dirs' is stored 'root'
                    int ret = connect_to_remote_server((files.empty() && (dirs.size() == 1)) || (mode == "RESTORE"), p);
                    if (ret < 0){
                        std::string error;
                        switch(ret){
                            case -1:
                                error = "database response error";
                                break;
                            case -2:
                                error = "wrong username or password";
                                break;
                            default:
                                error = "unknown error type";
                                break;
                        }
                        if((::write(p[1],error.c_str(),error.length()+1)) == -1){
                            if(errno != EPIPE){
                                throw std::runtime_error("cannot write on pipe");
                            }
                            // Else the pipe has been already closed, no problem
                            std::ostringstream error;
                            error << "minor problem in pipe writing: probably father already returned";
                            Log_Writer.writeLogAndClear(error);
                        }
                        throw general_exception(error);
                    }
                    std::string str = "connection succeed";
                    if((::write(p[1],str.c_str(),str.length()+1)) == -1){
                        if(errno != EPIPE){
                            throw std::runtime_error("cannot write on pipe");
                        }
                        // else the pipe has been already closed, no problem
                        std::ostringstream error;
                        error << "minor problem in pipe writing: probably father already returned";
                        Log_Writer.writeLogAndClear(error);
                    }
                    if (ret == RESTORE) {
                        restore(s, files, dirs, path, db_path, files.empty() && (dirs.size() == 1), root_ptr);
                    }
                    mode = "FETCH"; // set mode to FETCH after a restore

                    // SYN with server completed, starting to monitor client directory
                    synchronized = true;
                    std::thread t1([&thread_checker, &all_threads_running]() {
                        try {
                            fw.set(path, std::chrono::milliseconds(1000));
                            fw.setRunning(true);
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                            fw.start(modification_function);
                        } catch (std::exception& e) {
                            changeRunningState(all_threads_running,false,thread_checker);
                            std::ostringstream os;
                            os << e.what();
                            Log_Writer.writeLogAndClear(os);
                            fw.stop();
                        }
                    });
                    if (DEBUG)
                        std::cout << "--- System ready ---\n";
                    round_count = 0; // when the syncronisization is ended, reset the "try to connect" counter
                    std::thread t2([&thread_checker, &all_threads_running]() {
                        while (true) {
                            try {
                                if (!checkRunnigState(all_threads_running, thread_checker)) {
                                    fw.stop();
                                    return;
                                }
                                // every seconds check if there are some work to do or not
                                std::this_thread::sleep_for(std::chrono::seconds(1));
                                std::unique_lock<std::mutex> ul(action_server_mutex);
                                if (!fw.isRunning()) return;
                                ul.unlock();
                                action_on_server();
                            }
                            catch (std::exception& e) {
                                changeRunningState(all_threads_running,false,thread_checker);
                                std::ostringstream os;
                                os << e.what();
                                Log_Writer.writeLogAndClear(os);
                                fw.stop();
                                break;
                            }
                        }
                    });
                    t1.join();
                    t2.join();
                    if(checkRunnigState(all_threads_running,thread_checker))
                        return 0; // no errors, user asks to shutdown application
                    // if code arrives here it means that some error accours
                    throw socket_exception("no socket error, just retry"); // it's not a socket exception, but retry to restart application
                } catch (socket_exception &se) {
                    // reset variable and retry
                    resetVariables("socket_exc",se,thread_checker,all_threads_running);
                    root_ptr = nullptr;
                    synchronized = false;
                    std::this_thread::sleep_for(std::chrono::seconds(3)); // wait 3 seconds before reconnection
                } catch (general_exception &ge) {
                    resetVariables("general_exc",ge,thread_checker,all_threads_running);
                    files.clear();
                    return -1; // critical problem, redo the same thing doesn't resolve the problem
                } catch (std::exception &e) {
                    resetVariables("exc",e,thread_checker,all_threads_running);
                    return -1; // critical problem, redo the same thing doesn't resolve the problem
                }
            }
            return -1;
        }
        default: {
            close(p[1]);
            char auth_message[100];
            std::cout<<"child pid: "<<pid<<std::endl;
            int rc;
            do {
                if ((rc = ::read(p[0], auth_message, 100)) < 0)
                    throw std::runtime_error("error during pipe reading");
                if(rc == 0){
                    std::cout<<"general error. Open log.txt for more informations"<<std::endl;
                    return -1;
                }
                auth_message[rc] = '\0';
                std::cout << auth_message << std::endl;
                if (strcmp(auth_message, "user already connected") == 0)
                    return -2;
                if (strcmp(auth_message, "wrong username or password") == 0)
                    return -3;
            }while(strcmp(auth_message,"connection succeed") != 0);
            break;
        }
    }

    return 0;
}

void resetVariables(const std::string exc_type, const std::exception& e, std::mutex& m, bool& all_threads_running){
    changeRunningState(all_threads_running,false,m);
    fw.stop();
    std::ostringstream os;
    os << exc_type <<" "<< e.what();
    Log_Writer.writeLogAndClear(os);
    s.close();
    dirs.clear();
    files.clear();
}

int connect_to_remote_server(bool needs_restore, int* p){
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
    std::string server_digest;
    std::string client_digest = compute_db_digest(files,dirs);

    server_digest = connectRequest(s, username, password, mode);
    if(server_digest == "CONNECT-ERROR" || server_digest == "wrong username or password" || server_digest == "user already connected") {
        if(p != nullptr)
            ::write(p[1],server_digest.c_str(),server_digest.length()+1); // send to father the error result
        throw general_exception(server_digest); // stop the program
    }

    // CONNECT-OK
    if (!needs_restore){
        // check if the DB is updated
        if(!compareDigests(server_digest,client_digest)){
            if (DEBUG)
                std::cout<<"server DB is not updated\n";
            // get DB from server
            sendMsg(s,"GET-DB");
            rcvFile(s,server_db_path);
            // check which files and directories aren't updated
            checkDB(path,"",server_db_path,files,dirs,modification_function, root_ptr);
            if (DEBUG)
                std::cout<<"--- checkDB ended ---\n";
        } else {
            if (DEBUG)
                std::cout<<"server DB is updated\n";
            sendMsg(s,"Database up to date");
            if(rcvMsg(s) != "server_db_ok"){
                return -1; // error in db response on 'server_db_ok'
            }
        }
    }else{ // restore
        if(compareDigests(server_digest,client_digest)){ // if digest are the same no actions
            sendMsg(s, "Database up to date");
        } else {
            // server has some data, needs to restore
            return RESTORE;
        }
    }
    return UPDATED;
}

void action_on_server(){ // this function is used in a loop to check the state of the FileWatcher
    std::unique_lock<std::mutex> lg(action_server_mutex);
    if (!fw.isRunning()) return;
    FileWatcher_state last, cur;
    fw.getAllState(last, cur);

    if (!s.is_open() && cur == FileWatcher_state::mod_found &&
        last == FileWatcher_state::ready) { // first modification found, open the socket
        if (DEBUG)
            std::cout << "action_on_server opening" << std::endl;
        lg.unlock();
        connect_to_remote_server(false,
                                 nullptr); // this function can call 'action_on_server', deadlock without unlock
        lg.lock();
    }
    if (s.is_open() &&
        (cur == FileWatcher_state::ended ||
         cur == FileWatcher_state::ready && last == FileWatcher_state::ended)) {
        sendMsg(s, "update completed");
        s.close();
    }
}