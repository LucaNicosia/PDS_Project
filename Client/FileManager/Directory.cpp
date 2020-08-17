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

std::shared_ptr<Directory> Directory::addDirectory(std::string dName){

    if (this != nullptr){
        std::shared_ptr<Directory> newDir = makeDirectory(dName, self);
        dSons.push_back(newDir);
        fs::create_directories(newDir->path);
        return newDir;
    }else
        return nullptr;
}

std::shared_ptr<Directory> Directory::makeDirectory(std::string dName, std::weak_ptr<Directory> dFather){
    std::shared_ptr<Directory> newDir = std::make_shared<Directory>();
    newDir->path = dName;
    newDir->dFather = dFather;
    newDir->self = std::weak_ptr<Directory>(newDir);
    newDir->dSons = std::vector<std::shared_ptr<Directory>>();
    newDir->fSons = std::vector<std::shared_ptr<File>>();
    return newDir;
}

std::string Directory::toString(){
    return "Il nome della cartella è "+path;
}

std::shared_ptr<File> Directory::addFile (const File& ifile){

    if (this != nullptr){

        //File file {name, size, this->self};
        //std::shared_ptr<File> tmp (&file);
        std::shared_ptr<File> file = std::make_shared<File>(ifile);
        fSons.push_back(file);
        file->setDFather(this->self);
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
        if (name == dSons[i]->path){
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
            fSons.erase(fSons.begin()+i);
            return true;
        }
    }

    return false;
}

std::shared_ptr<Directory> Directory::getDir (const std::string& name){
    for (int i = 0; i < dSons.size(); i++){
        if (name == dSons[i]->path){
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

const std::weak_ptr<Directory> &Directory::getDFather() const {
    return dFather;
}

const std::weak_ptr<Directory> &Directory::getSelf() const {
    return self;
}

std::string Directory::getFatherFromPath(std::string path){
    return path.substr(0,path.find_last_of("/"));
}
