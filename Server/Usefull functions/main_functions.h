#ifndef MAIN_FUNCTIONS_H
#define MAIN_FUNCTIONS_H

#include <filesystem>
#include "Crypto/MyCryptoLibrary.h"
#include "../Entities/User/User.h"
#include "../Entities/Database/Database.h"
#include "utilities.h"
#include "constants.h"

int insertFileIntoDB(const std::string& db_path, std::shared_ptr<File>& file);
int deleteFileFromDB(const std::string& db_path, const std::shared_ptr<File>& file);
int updateFileDB(const std::string& db_path, std::shared_ptr<File>& file);
int deleteDirectoryFromDB(const std::string& db_path, const std::shared_ptr<Directory>& dir);
int insertDirectoryIntoDB(const std::string& db_path, std::shared_ptr<Directory>& dir);
std::string cleanPath(const std::string & path, const std::string& rubbish);
void printFilesAndDirs(std::map<std::string, std::shared_ptr<File>> files, std::map<std::string, std::shared_ptr<Directory>> dirs);
void check_user_data(const std::string& username_dir, const std::string& db_path);
void initialize_files_and_dirs(std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::string db_path, std::shared_ptr<Directory>& root, Socket& s);
std::string compute_db_digest(std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs);
void restore(Socket& s, const std::string& userPath, std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs);
void manageModification(Socket& s, std::string msg,const std::string& db_path, const std::string& userDirPath ,std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs);
int rcvConnectRequest(Socket& s, const std::string root_path, std::string& username, std::string& password, std::string& mode, std::shared_ptr<Directory>& root, std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::map<std::string, int>& users_connected, std::mutex& users_mutex, const int& id, std::mutex& db_mutex);
std::vector<User> selectUsers(const std::string& db_path, const std::string query, std::mutex& db_mutex);
bool insertUserIntoDB(const std::string& db_path, const User user, std::mutex& db_mutex);

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
    else if(!compareDigests(v[0].getHash(),file->getHash())) { // If present, update hash only
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

bool insertUserIntoDB(const std::string& db_path, const User user, std::mutex& db_mutex){
    std::lock_guard<std::mutex> lg(db_mutex);
    Database db(db_path);
    db.open();
    db.exec("INSERT INTO Users (username,password,salt) VALUES (\""+user.getUsername()+"\",\""+user.getPassword()+"\",\""+user.getSalt()+"\")");
    db.close();
    return true;
}

std::vector<User> selectUsers(const std::string& db_path, const std::string query, std::mutex& db_mutex){
    std::lock_guard<std::mutex> lg(db_mutex);
    std::vector<User> queryUsers;
    int nUsers;
    Database db(db_path);
    db.open();
    db.select(query,nUsers,queryUsers);
    db.close();
    return queryUsers;
}

std::string cleanPath(const std::string & path, const std::string& rubbish){ // eg. "TestPath/hello.txt" -> "hello.txt"
    std::string head = path;
    std::string tail;
    while(rubbish != head){
        tail = path.substr(head.find_last_of("/")+1);
        head = head.substr(0, head.find_last_of("/"));
    }
    return tail;
}

void printFilesAndDirs(std::map<std::string, std::shared_ptr<File>> files, std::map<std::string, std::shared_ptr<Directory>> dirs){
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
        // Username directory doesn't exists, create it
        std::filesystem::create_directories(username_dir);
    }
    std::ifstream db_file(db_path,std::ios::in);
    if(!db_file){
        std::ofstream file(db_path);
        Database db(db_path);
        // DB doesn't exists
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
    //DB doesn't exists
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

    std::thread t([&m,&running,&s](){ // Database updating may take lots of time. this trade is like an "heart beat"
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
        if(!std::filesystem::is_directory(server_path)){ // If directory is not in memory, remove it from DB
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
            // File doesn't exists, remove it from DB
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
    // Until now, files and dirs contains only db data. Check if data actually stored is right
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
    // If they are on db but not in memory, delete from db
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

void manageModification(Socket& s, std::string msg,const std::string& db_path, const std::string& userDirPath ,std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs){
    std::ostringstream os;
    std::string path;
    std::string name;
    std::string type;
    std::string operation;

    operation = msg.substr(msg.find_last_of(" ") + 1);
    msg = msg.substr(0, msg.find_last_of(" "));
    type = msg.substr(0, msg.find_first_of(" "));
    msg = msg.substr(msg.find_first_of(" ") + 1);
    path = msg;
    name = path.substr(path.find_last_of("/") + 1);

    std::weak_ptr<Directory> father = dirs[Directory::getFatherFromPath(path)];
    if (type == "FILE") {
        // File modification handler
        if (operation == "created") {
            sendMsg(s, "READY");
            std::string digest = rcvFile(s, userDirPath + "/" + path);
            sendMsg(s, "DONE");
            std::shared_ptr<File> file = father.lock()->addFile(name,
                                                                digest,
                                                                false);
            files[file->getPath()] = file;
            if (insertFileIntoDB(db_path, file) < 0) {
                os << "Problem in File insert on DB";
                Log_Writer.writeLogAndClear(os);
            }
        } else if (operation == "erased") {
            if (deleteFileFromDB(db_path, files[path]) < 0) {
                os << "Problem deleting File from DB";
                Log_Writer.writeLogAndClear(os);
            }
            father.lock()->removeFile(name);
            files.erase(path);
            sendMsg(s, "DONE");
        } else if (operation == "modified") {
            if (deleteFileFromDB(db_path, files[path]) < 0) {
                os << "Problem deleting File from DB";
                Log_Writer.writeLogAndClear(os);
            }
            father.lock()->removeFile(name);
            files.erase(path);
            sendMsg(s, "READY");
            std::string digest = rcvFile(s, userDirPath + "/" + path);
            sendMsg(s, "DONE");
            std::shared_ptr<File> file = father.lock()->addFile(name,
                                                                digest,
                                                                false);
            files[file->getPath()] = file;
            if (insertFileIntoDB(db_path, file)<0) {
                os << "Problem in File insert on DB";
                Log_Writer.writeLogAndClear(os);
            }
        } else {
            //Error
            os << "Wrong string format (" << type << " " << path << " "<< operation << ")";
            Log_Writer.writeLogAndClear(os);
            sendMsg(s, "ERROR");
        }
    } else if (type == "DIR") {
        // Dirs modification handler
        if (operation == "created") {
            std::shared_ptr<Directory> dir = father.lock()->addDirectory(name, true);
            dirs[dir->getPath()] = dir;
            if (insertDirectoryIntoDB(db_path, dir) < 0) {
                os << "Problem in Directory insert on DB";
                Log_Writer.writeLogAndClear(os);
            }
            sendMsg(s, "DONE");
        } else if (operation == "erased") {
            if (deleteDirectoryFromDB(db_path, dirs[path])<0) {
                os << "Problem deleting Directory from DB";
                Log_Writer.writeLogAndClear(os);
            }
            father.lock()->removeDir(name);
            dirs.erase(path);
            sendMsg(s, "DONE");
        } else if (operation == "modified") {
            // Nothing to do
        } else {
            //errore
            os << "Wrong string format (" << type << " " << path << " "<< operation << ")";
            Log_Writer.writeLogAndClear(os);
            sendMsg(s, "ERROR");
        }
    } else {
        throw std::runtime_error("unknown message type");
    }
}

int rcvConnectRequest(Socket& s, const std::string root_path, std::string& username, std::string& password, std::string& mode, std::shared_ptr<Directory>& root, std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::map<std::string, int>& users_connected, std::mutex& users_mutex, const int& id, std::mutex& db_mutex){
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
    for(auto it:users_connected){
        std::cout<<it.first<<":"<<it.second<<std::endl;
    }
    if(users_connected.count(username) >= 1){ // this user is already connected, refuse the connection
        rcvMsg(s); // only to sync
        sendMsg(s, "user already connected");
        return -2;
    }else{
        users_connected[username] = id; // add user to the map associated with his socket_id
    }
    ul.unlock();

    User user {username, password};

    std::string users_db_path = std::string(PATH_TO_DB)+"users/users.db";
    std::string db_path = std::string(PATH_TO_DB)+username+".db";
    std::ifstream users(users_db_path);
    if (!users){
        os<<"Creating user into DB"<<std::endl;
        Log_Writer.writeLogAndClear(os);
        createUsersDB(users_db_path);
    }

    queryUsers = selectUsers(users_db_path,"SELECT * FROM Users",db_mutex);

    for (User u : queryUsers){
        if (u.getUsername() == user.getUsername()){
            found = true;
            foundUser = u;
            break;
        }
    }

    if (!found){
        // User is not present on DB
        insertUserIntoDB(users_db_path, user,db_mutex);
        if(rcvMsg(s) == "CONNECT-OK"){
            root = std::make_shared<Directory>()->makeDirectory(root_path+"/"+username,std::weak_ptr<Directory>());
            check_user_data(root->getName(), db_path);
            initialize_files_and_dirs(files, dirs, db_path, root, s);
            std::string digest = compute_db_digest(files, dirs);
            sendMsg(s, "DIGEST " + digest);
        }
    }else{
        // User already present on DB
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
            rcvMsg(s); // Discard this message
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
    for(auto it:users_connected){
        std::cout<<it.first<<":"<<it.second<<std::endl;
    }
    users_connected.erase(user);
}

void addSocket(std::map<int,Socket>& sockets, int id, Socket& s, std::mutex& m){
    std::lock_guard<std::mutex>lg(m);
    sockets[id] = std::move(s);
}

#endif //MAIN_FUNCTIONS_H
