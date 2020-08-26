//
// Created by giuse on 27/04/2020.
//

#include "Directory.h"
#include "File.h"
#include <typeinfo>
#include <filesystem>
#include <fstream>

#define DIR 0
#define FILE 1


namespace fs = std::filesystem;

Directory::Directory(){

}

Directory::~Directory() {

}

std::shared_ptr<Directory> Directory::addDirectory(std::string dName, const bool& create_flag){

    if (this != nullptr){
        std::shared_ptr<Directory> newDir = makeDirectory(dName, self);
        dSons.push_back(newDir);
        if(create_flag) // create only when flag is true
            fs::create_directories(root.lock()->getName()+"/"+newDir->path);
        return newDir;
    }else{
        return nullptr;
    }

}

std::shared_ptr<Directory> Directory::makeDirectory(std::string dName, std::weak_ptr<Directory> dFather){
    std::shared_ptr<Directory> newDir = std::make_shared<Directory>();
    newDir->name = dName;
    newDir->dFather = dFather;
    newDir->self = newDir;
    newDir->dSons = std::vector<std::shared_ptr<Directory>>();
    newDir->fSons = std::vector<std::shared_ptr<File>>();
    if(dFather.expired()) {
        newDir->path = "";
        newDir->root = newDir->self;
        return newDir;
    }

    std::string path = newDir->dFather.lock()->path + "/" + dName;
    if (path.find_first_of("/") == 0) {
        newDir->path = path.substr(1); // delete the "/" at the beginning
    }else{
        newDir->path = path;
    }
    newDir->root = dFather.lock()->getRoot();
    return newDir;
}

std::shared_ptr<File> Directory::addFile (const std::string name, const std::string &hash, const bool& create_flag){

    if (this != 0){

        std::shared_ptr<File> file = std::make_shared<File>(name, hash, std::weak_ptr<Directory>(self));
        fSons.push_back(file);
        if(create_flag) // only when 'create_flag == true' the file is actually created
            std::ofstream(root.lock()->getName()+"/"+file->getPath());
        return file;
    }else
        return nullptr;

}

bool Directory::removeDir (const std::string& name){
    if (name == "..")
        return false;
    if (name == ".")
        return false;

    for (int i = 0; i < dSons.size(); i++){
        if (name == dSons[i]->name){
            fs::remove_all(root.lock()->getName()+"/"+dSons[i]->getPath());
            dSons.erase(dSons.begin()+i);
            return true;
        }
    }

    return false;
}

bool Directory::removeFile (const std::string& name){
    if (name == "..")
        return false;
    if (name == ".")
        return false;
    for (int i = 0; i < fSons.size(); i++){
        if (name == fSons[i]->getName()){
            fs::remove_all(root.lock()->getName()+"/"+fSons[i]->getPath());
            fSons.erase(fSons.begin()+i);
            return true;
        }
    }

    return false;
}

std::shared_ptr<Directory> Directory::getDir (const std::string& name){
    for (int i = 0; i < dSons.size(); i++){
        if (name == dSons[i]->name){
            return dSons[i];
        }
    }
    return nullptr;
}

std::shared_ptr<File> Directory::getFile (const std::string& name){
    for (int i = 0; i < fSons.size(); i++){
        if (name == fSons[i]->getPath()){
            return fSons[i];
        }
    }
    return nullptr;
}

void Directory::set(std::string field, std::string value){
    if(field == "id"){
        return; // id is not stored
    }else if(field == "path"){
        path = value;
    }else if(field == "name"){
        name = value;
    }else{
        std::cout<<"Directory: Invalid field! ("<<field<<")\n"; // QUI CI VUOLE UNA ECCEZIONE
    }
}

const std::string &Directory::getPath() const {
    return path;
}

void Directory::setPath(const std::string &path) {
    Directory::path = path;
}

const std::string &Directory::getName() const {
    return name;
}

void Directory::setName(const std::string &name) {
    Directory::name = name;
}

const std::weak_ptr<Directory> &Directory::getDFather() const {
    return dFather;
}

const std::vector <std::shared_ptr<Directory>> &Directory::getDSons() const {
    return dSons;
}

const std::vector <std::shared_ptr<File>> &Directory::getFSons() const {
    return fSons;
}

const std::weak_ptr<Directory> &Directory::getSelf() const {
    return self;
}

std::string Directory::getFatherFromPath(std::string path){
    return (path.find("/") == std::string::npos)?"":path.substr(0,path.find_last_of("/"));
}

void Directory::ls(int indent) const{
    std::string spaces;

    if (this != 0) {

        for (int i = 0; i < indent; i++) {
            spaces += " ";
        }
        if (this->name == root.lock()->getName())
            std::cout << spaces + this->name << std::endl;

        for (int i = 0; i < dSons.size(); i++) {
            std::cout << spaces + "   " + this->dSons[i]->name << std::endl;
            this->dSons[i]->ls(indent+4);
        }
        for (int i = 0; i < fSons.size(); i++) {
            std::cout << spaces + "   " + this->fSons[i]->getName() << std::endl;
        }
    }

}

std::string Directory::toString (){
    return "PATH = "+path+" NAME = "+name+" ROOT_PATH = "+root.lock()->getPath()+" ROOT_NAME = "+root.lock()->getName();
}

std::weak_ptr<Directory> Directory::getRoot() {
    return root;
}

std::weak_ptr<Directory> Directory::setRoot(std::weak_ptr<Directory> root){
    this->root = root;
    return root;
}
