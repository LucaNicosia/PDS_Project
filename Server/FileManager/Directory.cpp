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

#define ROOT "server_directory"

namespace fs = std::filesystem;

Directory::Directory(){

}

Directory::~Directory() {

}

std::shared_ptr<Directory> Directory::addDirectory(std::string dName){

    if (this != nullptr){
        std::shared_ptr<Directory> newDir = makeDirectory(dName, self);
        dSons.push_back(newDir);
        fs::create_directories(newDir->path);
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
    if (dName != ROOT)
        newDir->path = newDir->dFather.lock()->path+"/"+dName;
    else
        newDir->path = ROOT;
    return newDir;
}

std::shared_ptr<File> Directory::addFile (const std::string name, const std::string &hash){

    if (this != 0){

        //File file {name, size, this->self};
        //std::shared_ptr<File> tmp (&file);
        std::shared_ptr<File> file = std::make_shared<File>(path+"/"+name, hash, std::weak_ptr<Directory>(self));
        fSons.push_back(file);
        if (this->name != ROOT)
        file->setPath(file->getDFather().lock()->path+"/"+file->getPath());
        else {
            file->setPath(file->getFatherPath()+"/"+file->getPath());
        }
        //std::ofstream(file->getPath());
        //std::cout<<"Dentro add file "<<this->fSons[0]<<" fileName = "<<this->fSons[0]->getName()<<std::endl;
        return file;
    }else
        return nullptr;

}

bool Directory::renameDir (const std::string& oldName, const std::string& newName){
    if (oldName == ".." || newName == "..")
        return false;
    if (oldName == "." || newName == ".")
        return false;

    for (int i = 0; i < dSons.size(); i++){
        if (oldName == dSons[i]->name){
            dSons[i]->name = newName;
            dSons[i]->path = dSons[i]->dFather.lock()->path+"/"+newName;
            fs::rename(dSons[i]->dFather.lock()->path+"/"+oldName, dSons[i]->path);
            return true;
        }
    }

    return false;
}

bool Directory::renameFile (const std::string& oldName, const std::string& newName){
    if (oldName == ".." || newName == "..")
        return false;
    if (oldName == "." || newName == ".")
        return false;

    for (int i = 0; i < fSons.size(); i++){
        if (oldName == fSons[i]->getName()){
            fSons[i]->setPath(fSons[i]->getDFather().lock()->path+"/"+newName);
            fs::rename(fSons[i]->getDFather().lock()->path+"/"+oldName, fSons[i]->getPath());
            return true;
        }
    }

    return false;

}

bool Directory::removeDir (const std::string& name){
    if (name == "..")
        return false;
    if (name == ".")
        return false;

    for (int i = 0; i < dSons.size(); i++){
        if (name == dSons[i]->name){
            std::uintmax_t n = fs::remove_all(dSons[i]->getPath());
            std::cout << "Deleted " << n << " files or directories"<<std::endl;
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
        if (name == fSons[i]->getPath()){
            std::uintmax_t n = fs::remove_all(fSons[i]->getPath());
            std::cout << "Deleted " << n << " files"<<std::endl;
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
        std::cout<<"Invalid field!\n"; // QUI CI VUOLE UNA ECCEZIONE
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

const std::weak_ptr<Directory> &Directory::getSelf() const {
    return self;
}

std::string Directory::getFatherFromPath(std::string path){
    return path.substr(0,path.find_last_of("/"));
}

std::shared_ptr<Directory> Directory::getRoot() {
    if (root == std::shared_ptr<Directory>())
        root = makeDirectory(ROOT, std::weak_ptr<Directory>());
    return root;
}

void Directory::ls(int indent) const{
    std::string spaces;

    if (this != 0) {

        for (int i = 0; i < indent; i++) {
            spaces += " ";
        }
        if (this->name == ROOT)
         std::cout << spaces + this->name << std::endl;

        for (int i = 0; i < dSons.size(); i++) {
            std::cout << spaces + "   " + this->dSons[i]->name << std::endl;
            this->dSons[i]->ls(indent+4);
        }
        for (int i = 0; i < fSons.size(); i++) {
            //std::cout<<"Dentro ls file "<<this->fSons[i]<<" fileName = "<<this->fSons[i]->getName()<<std::endl;
            std::cout << spaces + "   " + this->fSons[i]->getName() << std::endl;
        }
    }

}