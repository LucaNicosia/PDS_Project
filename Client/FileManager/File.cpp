//
// Created by giuse on 27/04/2020.
//

#include "File.h"


//Costruttore

File::File() {}

File::File(const std::string path, int id, int id_dir, const std::string &hash, std::weak_ptr<Directory> dFather) {
    this->path = path;
    this->hash = hash;
    this->id = id;
    this->id_dir = id_dir;
    this->dFather = dFather;
}

int File::getId() const {
    return id;
}

void File::setId(int id) {
    File::id = id;
}

int File::getIdDir() const {
    return id_dir;
}

void File::setIdDir(int idDir) {
    id_dir = idDir;
}

const std::string &File::getHash() const {
    return hash;
}

void File::setHash(const std::string &hash) {
    File::hash = hash;
}

const std::weak_ptr<Directory> &File::getDFather() const {
    return dFather;
}

void File::setDFather(const std::weak_ptr<Directory> &dFather) {
    File::dFather = dFather;
}

void File::set(std::string field, std::string value) {
    if(field == "id"){
        id = std::atoi(value.c_str());
    }else if(field == "id_dir"){
        id_dir = std::atoi(value.c_str());
    }else if(field == "nome"){
        path = value;
    }else if(field == "hash"){
        hash = value;
    }else{
        std::cout<<"Invalid field!\n"; // QUI CI VUOLE UNA ECCEZIONE
    }
}

std::string File::getFatherPath(){
    if(this->dFather.expired())
        return "";
    return this->dFather.lock()->getPath();
}

const std::string &File::getPath() const {
    return path;
}

void File::setPath(const std::string &path) {
    File::path = path;
}
