//
// Created by root on 14/08/20.
//

#ifndef PDS_PROJECT_SERVER_MAIN_FUNCTIONS_H
#define PDS_PROJECT_SERVER_MAIN_FUNCTIONS_H

bool insertFileIntoDB(const std::string& db_path, std::shared_ptr<File>& file){
    Database db(db_path);
    std::ifstream db_file(db_path);
    if(!db_file){
        // a file that doesn't exits
        return false;
    }
    db.open();

    db.exec("INSERT INTO FILE (path,hash,name) VALUES (\""+file->getPath()+"\",\""+file->getHash()+"\",\""+file->getName()+"\")");

    db.close();
    return true;
}
bool deleteFileFromDB(const std::string& db_path, const std::shared_ptr<File>& file){
    Database db(db_path);
    std::ifstream db_file(db_path);

    if(!db_file){
        // a file that doesn't exits
        return false;
    }
    db.open();

    db.exec("DELETE FROM FILE WHERE path = \""+file->getPath()+"\"");

    db.close();
    return true;
}
bool updateFileDB(const std::string& db_path, std::shared_ptr<File>& file){
    Database db(db_path);
    std::ifstream db_file(db_path);

    if(!db_file){
        // a file that doesn't exits
        return false;
    }
    db.open();

    db.exec("UPDATE FILE SET hash = \""+file->getHash()+"\" WHERE path = \""+file->getPath()+"\"");

    db.close();
    return true;
}

bool insertDirectoryIntoDB(const std::string& db_path, std::shared_ptr<Directory>& dir){
    Database db(db_path);
    std::ifstream db_file(db_path);

    if(!db_file){
        // a file that doesn't exits
        return false;
    }
    db.open();

    db.exec("INSERT INTO DIRECTORY (path,name) VALUES (\""+dir->getPath()+"\", "+"\""+dir->getName()+"\")");

    db.close();
    return true;
}
bool deleteDirectoryFromDB(const std::string& db_path, const std::shared_ptr<Directory>& dir){
    Database db(db_path);
    std::ifstream db_file(db_path);

    if(!db_file){
        // a file that doesn't exits
        return false;
    }
    db.open();

    db.exec("DELETE FROM DIRECTORY WHERE path = \""+dir->getPath()+"\"");

    for (int i = 0; i < dir->getDSons().size(); i++) {
        deleteDirectoryFromDB(db_path, dir->getDSons()[i]);
    }
    for (int i = 0; i < dir->getFSons().size(); i++) {
        deleteFileFromDB(db_path, dir->getFSons()[i]);
    }

    db.close();
    return true;
}

std::string cleanPath(const std::string & path, const std::string& rubbish){ // es. "TestPath/ciao.txt" -> "ciao.txt"
    std::string head = path;
    std::string tail;
    //std::cout<<"rubbish: "<<rubbish<<"\npath: "<<path<<"\n";
    while(rubbish != head){
        tail = path.substr(head.find_last_of("/")+1);
        head = head.substr(0, head.find_last_of("/"));
        //std::cout<<"head: "<<head<<"\ntail: "<<tail<<"\n";
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
    Database db(db_path);
    if(!db_file){
        // database doesn't exists
        db.open();
        db.exec("CREATE TABLE \"DIRECTORY\" ("
                "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT, "
                "\"path\" TEXT NOT NULL UNIQUE,"
                "\"name\" TEXT NOT NULL"
                ")");
        db.exec("CREATE TABLE \"FILE\" ("
                "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT, "
                "\"path\" TEXT NOT NULL UNIQUE, "
                "\"name\" TEXT NOT NULL,"
                "\"hash\" TEXT NOT NULL"
                ")");
    }
}

void initialize_files_and_dirs(std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::string path, std::string db_path, std::shared_ptr<Directory> root){
    Database db(db_path);
    int nFiles, nDirs;
    std::vector<File> queryFiles;
    std::vector<Directory> queryDirs;

    dirs[""] = root;

    db.open();

    db.select("SELECT * FROM File",nFiles,queryFiles);
    db.select("SELECT * FROM Directory",nDirs,queryDirs);

    db.close();
    for (int i = 0; i < nDirs; i++){
        std::string server_path = path + "/" + queryDirs[i].getPath();
        std::cout<<server_path<<" --- "<<Directory::getFatherFromPath(server_path)<<"\n";
        std::weak_ptr<Directory> father;
        father = dirs[Directory::getFatherFromPath(queryDirs[i].getPath())]->getSelf();
        std::shared_ptr<Directory> dir = father.lock()->addDirectory(queryDirs[i].getName(), !std::filesystem::is_directory(server_path)); // if it is already a directory, it is not created
        dirs[dir->getPath()] = dir;
    }

    for (int i = 0; i < nFiles; i++){
        std::size_t found = queryFiles[i].getPath().find_last_of("/");
        queryFiles[i].setName(queryFiles[i].getPath().substr(found+1));
        std::weak_ptr<Directory> father = dirs[Directory::getFatherFromPath(queryFiles[i].getPath())]->getSelf();
        std::shared_ptr<File> file = father.lock()->addFile(queryFiles[i].getName(), queryFiles[i].getHash(), false);
        files[file->getPath()] = file;
    }

    // files and dirs contains only db data. Check if data actually stored is right
    
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
        if(!std::filesystem::exists(std::filesystem::status(path + "/" + it.second->getPath()))){
            deleteDirectoryFromDB(db_path,it.second);
            it.second->getDFather().lock()->removeDir(it.second->getName());
            dirs.erase(it.first);
        }
    }
    stampaFilesEDirs(files, dirs);
    root->ls(4);
}

#endif //PDS_PROJECT_SERVER_MAIN_FUNCTIONS_H
