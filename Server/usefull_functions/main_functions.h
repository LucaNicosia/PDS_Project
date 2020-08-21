//
// Created by root on 14/08/20.
//

#ifndef PDS_PROJECT_SERVER_MAIN_FUNCTIONS_H
#define PDS_PROJECT_SERVER_MAIN_FUNCTIONS_H



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

void initialize_files_and_dirs(std::map<std::string, std::shared_ptr<File>>& files, std::map<std::string, std::shared_ptr<Directory>>& dirs, std::string path, std::string db_path){

    Database db(db_path);
    int nFiles, nDirs;
    std::vector<File> queryFiles;
    std::vector<Directory> queryDirs;

    Directory::setRoot(path);
    std::shared_ptr<Directory> root = Directory::getRoot();
    dirs[root->getPath()] = root;

    db.open();

    db.select("SELECT * FROM File",nFiles,queryFiles);
    db.select("SELECT * FROM Directory",nDirs,queryDirs);


    for (int i = 0; i < nDirs; i++){
        std::weak_ptr<Directory> father;
        father = dirs[Directory::getFatherFromPath(queryDirs[i].getPath())]->getSelf();
        std::shared_ptr<Directory> dir = father.lock()->addDirectory(queryDirs[i].getName(), false);
        dirs[dir->getPath()] = dir;
    }

    for (int i = 0; i < nFiles; i++){
        std::size_t found = queryFiles[i].getPath().find_last_of("/");
        queryFiles[i].setName(queryFiles[i].getPath().substr(found+1));
        std::weak_ptr<Directory> father = dirs[Directory::getFatherFromPath(queryFiles[i].getPath())]->getSelf();
        std::shared_ptr<File> file = father.lock()->addFile(queryFiles[i].getName(), queryFiles[i].getHash(), false);
        files[file->getPath()] = file;
    }

    stampaFilesEDirs(files, dirs);
    root->ls(4);
}

#endif //PDS_PROJECT_SERVER_MAIN_FUNCTIONS_H
