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

std::shared_ptr<Directory> Directory::addDirectory(std::string dName, int id){

    if (this != nullptr){
        std::shared_ptr<Directory> newDir = makeDirectory(id, dName, self);
        dSons.push_back(newDir);
        fs::create_directories(newDir->path);
        return newDir;
    }else
        return nullptr;
}

std::shared_ptr<Directory> Directory::makeDirectory(int id, std::string dName, std::weak_ptr<Directory> dFather){
    std::shared_ptr<Directory> newDir = std::make_shared<Directory>();
    newDir->name = dName;
    newDir->path = dName;
    newDir->id = id;
    newDir->dFather = dFather;
    newDir->self = std::weak_ptr<Directory>(newDir);
    newDir->dSons = std::vector<std::shared_ptr<Directory>>();
    newDir->fSons = std::vector<std::shared_ptr<File>>();
    /* TODO: controllare perchè non funziona
    if (dName != "root")
        newDir->path = newDir->dFather.lock()->path+"/"+dName;
    else
        newDir->path = "root";
    */
    return newDir;
}

std::string Directory::toString(){
    return "Il nome della cartella è "+name;
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
        if (name == dSons[i]->name){
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
        id = std::atoi(value.c_str());
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

int Directory::getId() const {
    return id;
}

void Directory::setId(int id) {
    Directory::id = id;
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
