//
// Created by giuse on 27/04/2020.
//

#include "File.h"


//Costruttore

File::File() {
    path = "";
    hash = "";
}

File::File(const std::string path, const std::string &hash, std::weak_ptr<Directory> dFather) {
    this->path = path;
    this->hash = hash;
    this->dFather = dFather;
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
        path = in.getPath();
        hash = in.getHash();
    }
    return *this;
}

void File::set(const std::string& field, const std::string& value) {
    if(field == "id" || field == "id_dir"){
        return; // id are not saved on File
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
        path = other.getPath();
        hash = other.getHash();
    }
}

std::string File::toString() {
    std::string str = path+" "+hash;
    return str;
}
