//
// Created by root on 14/08/20.
//

#ifndef PDS_PROJECT_SERVER_MAIN_FUNCTIONS_H
#define PDS_PROJECT_SERVER_MAIN_FUNCTIONS_H

#include "Crypto/MyCryptoLibrary.h"
#include "../Entities/User/User.h"
#include "utilities.h"

#define PATH_TO_DB "../DB/"
#define LOG_PATH "../Log/log.txt"
#define TMP_LOG_FILE "../Log/tmp_log.txt" // not used for now
#define SERVER_DIRECTORY "server_directory"

int insertFileIntoDB(const std::string& db_path, std::shared_ptr<File>& file);
int deleteFileFromDB(const std::string& db_path, const std::shared_ptr<File>& file);
int updateFileDB(const std::string& db_path, std::shared_ptr<File>& file);
int deleteDirectoryFromDB(const std::string& db_path, const std::shared_ptr<Directory>& dir);
int insertDirectoryIntoDB(const std::string& db_path, std::shared_ptr<Directory>& dir);
std::string cleanPath(const std::string & path, const std::string& rubbish);
void stampaFilesEDirs(std::map<std::string, std::shared_ptr<File>> files, std::map<std::string, std::shared_ptr<Directory>> dirs);
void check_user_data(const std::string& username_dir, const std::string& db_path);
void initialize_files_and_dirs(std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::string db_path, std::shared_ptr<Directory>& root, Socket& s);
std::string compute_db_digest(std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs);
void restore(Socket& s, const std::string& userPath, std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs);
void manageModification(Socket& s, std::string msg,const std::string& db_path, const std::string& userDirPath ,std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::mutex& log_mutex);
int rcvConnectRequest(Socket& s, const std::string root_path, std::string& username, std::string& password, std::string& mode, std::shared_ptr<Directory>& root, std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::map<std::string, int>& users_connected, std::mutex& users_mutex, std::mutex& log_mutex);

int insertFileIntoDB(const std::string& db_path, std::shared_ptr<File>& file){
    Database db(db_path);

    db.open();
    std::vector<File> v;
    int n;
    db.select("SELECT * FROM FILE WHERE path = \""+file->getPath()+"\"",n,v);
    if(n == 0) { // new element
        db.exec("INSERT INTO FILE (path,hash,name) VALUES (\"" + file->getPath() + "\",\"" + file->getHash() + "\",\"" +file->getName() + "\")");
        return 0;
    }
    else if(!compareDigests(v[0].getPath(),file->getPath())) { // if present, update hash only
        updateFileDB(db_path,file);
        return 1;
    }

    db.close();
    return true;
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

bool insertUserIntoDB(const std::string& db_path, const User user){
    Database db(db_path);

    db.open();

    db.exec("INSERT INTO Users (username,password,salt) VALUES (\""+user.getUsername()+"\",\""+user.getPassword()+"\",\""+user.getSalt()+"\")");

    db.close();
    return true;
}

std::string cleanPath(const std::string & path, const std::string& rubbish){ // es. "TestPath/ciao.txt" -> "ciao.txt"
    std::string head = path;
    std::string tail;
    while(rubbish != head){
        tail = path.substr(head.find_last_of("/")+1);
        head = head.substr(0, head.find_last_of("/"));
    }
    return tail;
}

void stampaFilesEDirs(std::map<std::string, std::shared_ptr<File>> files, std::map<std::string, std::shared_ptr<Directory>> dirs){
    std::cout<<"***DIRECTORIES***"<<std::endl;
    for (const auto& x : dirs) {
        std::cout << x.first << ": " << x.second->toString()<< std::endl;
    }

    std::cout<<"***FILES***"<<std::endl;
    for (const auto& x : files) {
        std::cout << x.first << ": " << x.second->toString()<< std::endl;
    }
}

void check_user_data(const std::string& username_dir, const std::string& db_path){
    if(!std::filesystem::is_directory(username_dir)){
        // username directory doesn't exists, create it
        std::filesystem::create_directories(username_dir);
    }
    std::ifstream db_file(db_path,std::ios::in);
    if(!db_file){
        std::ofstream file(db_path);
        Database db(db_path);
        // database doesn't exists
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
    }
}

void createUsersDB(const std::string& db_path){
    std::ofstream db_file(db_path);
    if(!db_file.is_open())
        throw std::runtime_error("unable to create file");
    db_file.close();
    Database db(db_path);
    // database doesn't exists
    db.open();
    db.exec("CREATE TABLE \"Users\" ("
            "\"username\" TEXT PRIMARY KEY NOT NULL UNIQUE,"
            "\"password\" TEXT NOT NULL,"
            "\"salt\" TEXT NOT NULL"
            ")");

}

void initialize_files_and_dirs(std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::string db_path, std::shared_ptr<Directory>& root, Socket& s){
    Database db(db_path);
    int nFiles, nDirs;
    std::vector<File> queryFiles;
    std::vector<Directory> queryDirs;
    std::string path = root->getName();

    std::mutex m;
    bool running = true;

    std::thread t([&m,&running,&s](){ // database updating may take lots of time. this trade is like an "heart beat"
        while(true){
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::lock_guard<std::mutex>lg(m);
            if(!running) return;
            sendMsg(s,"updating database");
        }
    });

    dirs[""] = root;

    db.open();

    db.select("SELECT * FROM File",nFiles,queryFiles);
    db.select("SELECT * FROM Directory",nDirs,queryDirs);

    db.close();
    for (int i = 0; i < nDirs; i++){
        std::string server_path = path + "/" + queryDirs[i].getPath();
        if(!std::filesystem::is_directory(server_path)){ // if directory is not in memory, remove it from DB
            deleteDirectoryFromDB(db_path,std::make_shared<Directory>(queryDirs[i]));
            continue;
        }
        std::weak_ptr<Directory> father;
        father = dirs[Directory::getFatherFromPath(queryDirs[i].getPath())]->getSelf();
        std::shared_ptr<Directory> dir = father.lock()->addDirectory(queryDirs[i].getName(), !std::filesystem::is_directory(server_path)); // if it is already a directory, it is not created
        dirs[dir->getPath()] = dir;
    }

    for (int i = 0; i < nFiles; i++){
        std::string server_path = path + "/" + queryFiles[i].getPath();
        std::size_t found = queryFiles[i].getPath().find_last_of("/");
        queryFiles[i].setName(queryFiles[i].getPath().substr(found+1));
        if(!std::filesystem::exists(std::filesystem::status(server_path))){
            // file doesn't exists, remove it from DB
            deleteFileFromDB(db_path,std::make_shared<File>(queryFiles[i]));
            continue;
        }
        else{
            std::weak_ptr<Directory> father = dirs[Directory::getFatherFromPath(queryFiles[i].getPath())]->getSelf();
            std::shared_ptr<File> file;
            file = father.lock()->addFile(queryFiles[i].getName(), queryFiles[i].getHash(),false);
            files[file->getPath()] = file;
        }
    }
    // until now, files and dirs contains only db data. Check if data actually stored is right
    // if in memory but not on db, delete from memory
    std::vector<std::string> path_to_delete;
    for(auto it : std::filesystem::recursive_directory_iterator(path)){
        if(dirs.count(cleanPath(it.path().string(),path)) == 0 && files.count(cleanPath(it.path().string(),path)) == 0){ // if it is not stored in db, delete it
            path_to_delete.push_back(it.path().string());
        }
    }
    for(auto it : path_to_delete){
        std::filesystem::remove_all(it);
    }
    // if they are on db but not in memory, delete from db
    for(auto it : files){
        if(!std::filesystem::exists(std::filesystem::status(path + "/" + it.second->getPath()))){
            deleteFileFromDB(db_path,it.second);
            it.second->getDFather().lock()->removeFile(it.second->getName());
            files.erase(it.first);
        }
    }
    for(auto it : dirs){
        if(!std::filesystem::is_directory(path + "/" + it.second->getPath())){
            deleteDirectoryFromDB(db_path,it.second);
            it.second->getDFather().lock()->removeDir(it.second->getName());
            dirs.erase(it.first);
        }
    }


    std::unique_lock<std::mutex>lg(m);
    running = false;
    lg.unlock();
    t.join();
}

// server and client may have different order on db, compute the digest manually
std::string compute_db_digest(std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs){
    // files and dirs contains data of db and also stored in memory
    for(auto it : files){
        appendDigest(it.second->toString());
    }
    for(auto it : dirs){
        if(it.second->getPath() == "") // root is not included in db
            continue;
        appendDigest(it.second->toString());
    }
    return getAppendedDigest();
}

void restore(Socket& s, const std::string& userPath, std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs){
    for (const auto& it : dirs){
        if(it.second->getPath() == "")
            continue; // root is not sent
        sendMsg(s,"DIR "+it.second->getPath()+" created");
        if(rcvMsg(s) == "DONE")
            continue;
        throw "error in directories restoring";
    }

    for (const auto& it : files) {
        sendMsg(s,"FILE "+it.second->getPath()+" created");
        if(rcvMsg(s) == "READY"){
            int ret = sendFile(s,userPath+"/"+it.second->getPath(),it.second->getPath());
            if(ret == 1){
                continue;
            }
            if(ret == 0 && rcvMsg(s) == "DONE"){
                continue;
            }
        }
        throw "error in files restoring";
    }

    sendMsg(s,"restore completed");
}

void manageModification(Socket& s, std::string msg,const std::string& db_path, const std::string& userDirPath ,std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::mutex& log_mutex){
    std::ostringstream os;
    std::string path;
    std::string name;
    std::string type;
    std::string operation;
    // get data from message 'msg'
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
            rcvFile(s, userDirPath + "/" + path);
            sendMsg(s, "DONE");
            std::shared_ptr<File> file = father.lock()->addFile(name,
                                                                computeDigest(userDirPath + "/" + path),
                                                                false);
            files[file->getPath()] = file;
            if (insertFileIntoDB(db_path, file) < 0) {
                os << "Problema nell'inserire il file sul DB" << std::endl;
                writeLogAndClear(os,LOG_PATH,log_mutex);
            }
        } else if (operation == "erased") {
            if (deleteFileFromDB(db_path, files[path]) < 0) {
                os << "Problema nel cancellare il file sul DB" << std::endl;
                writeLogAndClear(os,LOG_PATH,log_mutex);
            }
            father.lock()->removeFile(name);
            files.erase(path);
            sendMsg(s, "DONE");
        } else if (operation == "modified") {
            if (deleteFileFromDB(db_path, files[path]) < 0) {
                os << "Problema nel cancellare il file sul DB" << std::endl;
                writeLogAndClear(os,LOG_PATH,log_mutex);
            }
            father.lock()->removeFile(name);
            files.erase(path);
            sendMsg(s, "READY");
            rcvFile(s, userDirPath + "/" + path);
            sendMsg(s, "DONE");
            std::shared_ptr<File> file = father.lock()->addFile(name,
                                                                computeDigest(userDirPath + "/" + path),
                                                                false);
            files[file->getPath()] = file;
            if (insertFileIntoDB(db_path, file)<0) {
                os << "Problema nell'inserire il file sul DB" << std::endl;
                writeLogAndClear(os,LOG_PATH,log_mutex);
            }
        } else {
            //errore
            os << "Stringa non ricevuta correttamente (" << type << " " << path << " "
                      << operation << ")" << std::endl;
            writeLogAndClear(os,LOG_PATH,log_mutex);
            sendMsg(s, "ERROR");
        }
    } else if (type == "DIR") {
        //dirs modification handler
        if (operation == "created") {
            std::shared_ptr<Directory> dir = father.lock()->addDirectory(name, true);
            dirs[dir->getPath()] = dir;
            if (insertDirectoryIntoDB(db_path, dir) < 0) {
                os << "Problema nell'inserire la directory sul DB" << std::endl;
                writeLogAndClear(os,LOG_PATH,log_mutex);
            }
            sendMsg(s, "DONE");
        } else if (operation == "erased") {
            if (deleteDirectoryFromDB(db_path, dirs[path])<0) {
                os << "Problema nel cancellare la directory sul DB" << std::endl;
                writeLogAndClear(os,LOG_PATH,log_mutex);
            }
            father.lock()->removeDir(name);
            dirs.erase(path);
            sendMsg(s, "DONE");
        } else if (operation == "modified") {
            // nothing to do
        } else {
            //errore
            os << "Stringa non ricevuta correttamente (" << type << " " << path << " "
                      << operation << ")" << std::endl;
            writeLogAndClear(os,LOG_PATH,log_mutex);
            sendMsg(s, "ERROR");
        }
    } else {
        throw std::runtime_error("unknown message type");
    }
}

int rcvConnectRequest(Socket& s, const std::string root_path, std::string& username, std::string& password, std::string& mode, std::shared_ptr<Directory>& root, std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::map<std::string, int>& users_connected, std::mutex& users_mutex, std::mutex& log_mutex) {
    std::ostringstream os;
    std::string msg = rcvMsg(s);
    sendMsg(s,"CONNECT-OK");
    std::string delimiter = " ";
    size_t pos = 0;
    std::string token;
    int i = 0;
    int nUsers;
    std::vector<User> queryUsers;
    bool found = false;
    User foundUser;

    while ((pos = msg.find(delimiter)) != std::string::npos) {
        token = msg.substr(0, pos);
        if (i == 1){
            username = token;
        }else if (i == 2){
            password = token;
        }
        msg.erase(0, pos + delimiter.length());
        i++;
    }
    mode = msg;
    std::unique_lock<std::mutex> ul(users_mutex);
    if(users_connected.count(username) == 1){ // this user is already connected, refuse the connection
        sendMsg(s, "user already connected");
        return -2;
    }
    ul.unlock();

    User user {username, password};

    std::string users_db_path = std::string(PATH_TO_DB)+"users/users.db";
    std::string db_path = std::string(PATH_TO_DB)+username+".db";
    std::ifstream users(users_db_path);
    if (!users){
        os<<"creating user DB"<<std::endl;
        writeLogAndClear(os,LOG_PATH,log_mutex);
        createUsersDB(users_db_path);
    }

    Database db(users_db_path);

    db.open();
    db.select("SELECT * FROM Users",nUsers,queryUsers);
    db.close();

    for (User u : queryUsers){
        if (u.getUsername() == user.getUsername()){
            found = true;
            foundUser = u;
            break;
        }
    }

    if (!found){
        // User not present on the db
        insertUserIntoDB(users_db_path, user);
        if(rcvMsg(s) == "CONNECT-OK"){
            root = std::make_shared<Directory>()->makeDirectory(root_path+"/"+username,std::weak_ptr<Directory>());
            check_user_data(root->getName(), db_path);
            initialize_files_and_dirs(files, dirs, db_path, root, s);
            std::string digest = compute_db_digest(files, dirs);
            sendMsg(s, "DIGEST " + digest);
        }
    }else{
        // User already present on the db
        if (foundUser.getPassword() == computePasswordDigest(password+foundUser.getSalt())){
            // Correct username and password
            if(rcvMsg(s) == "CONNECT-OK"){
                root = std::make_shared<Directory>()->makeDirectory(root_path+"/"+username,std::weak_ptr<Directory>());
                check_user_data(root->getName(), db_path);
                initialize_files_and_dirs(files, dirs, db_path, root, s);
                std::string digest = compute_db_digest(files, dirs);
                sendMsg(s, "DIGEST " + digest);
            }
            return 0;
        }else{
            // Wrong username and/or password
            rcvMsg(s); // discard this message
            sendMsg(s, "wrong username or password");
            return -1;
        }
    }

    return 0;
}

void eraseSocket(std::map<int,Socket>& sockets, int id, std::mutex& m){
    std::lock_guard<std::mutex>lg(m);
    sockets.erase(id);
}

void eraseUser(std::map<std::string, int>& users_connected, std::string user, std::mutex& m){
    std::lock_guard<std::mutex>lg(m);
    users_connected.erase(user);
}

void addSocket(std::map<int,Socket>& sockets, int id, Socket& s, std::mutex& m){
    std::lock_guard<std::mutex>lg(m);
    sockets[id] = std::move(s);
}

#endif //PDS_PROJECT_SERVER_MAIN_FUNCTIONS_H
