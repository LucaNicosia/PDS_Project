//
// Created by giuse on 27/04/2020.
//

#include "File.h"


//Costruttore

File::File() {
    id = -1;
    id_dir = -1;
    path = "";
    hash = "";
}

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

File& File::operator=(const File& in){
    if(&in != this){
        id = in.getId();
        path = in.getPath();
        id_dir = in.getIdDir();
        hash = in.getHash();
    }
    return *this;
}

void File::set(const std::string& field, const std::string& value) {
    if(field == "id"){
        id = (int)std::strtol(value.c_str(),nullptr,10);
    }else if(field == "id_dir"){
        id_dir = (int)std::strtol(value.c_str(),nullptr,10);;
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

File::File(const File &other) {
    if(&other != this){
        id = other.getId();
        path = other.getPath();
        id_dir = other.getIdDir();
        hash = other.getHash();
    }
}

std::string File::toString() {
    std::string str = id+" "+path+" "+hash;
    return str;
}
