//
// Created by giuse on 27/04/2020.
//

#include "File.h"


//Costruttore

File::File() {
    path = "";
    hash = "";
}

File::File(const std::string name, const std::string &hash, std::weak_ptr<Directory> dFather) {
    std::cout<<"File: "<<name<<"\n";
    this->name = name;
    this->path = dFather.lock()->getPath()+"/"+name;
    this->hash = hash;
    this->dFather = dFather;
}

const std::string &File::getHash() const {
    return hash;
}

const std::string &File::getName() const {
    return name;
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
    }else if(field == "path"){
        path = value;
    }else if(field == "hash"){
        hash = value;
    }else{
        std::cout<<"File: Invalid field! ("<<field<<")\n"; // QUI CI VUOLE UNA ECCEZIONE
    }
}

std::string File::getFatherPath(){
    if(this->dFather.expired())
        return "./";
    return this->dFather.lock()->getPath();
}

const std::string &File::getPath() const {
    return path;
}

void File::setPath(const std::string &path) {
    File::path = path;
    std::size_t found = path.find_last_of("/");
    this->name = this->path.substr(found+1);
}

File::File(const File &other) {
    if(&other != this){
        path = other.getPath();
        hash = other.getHash();
        dFather = other.getDFather();
    }
}

void File::ls (int indent) const{

    std::string spaces;

    if (this != 0) {

        for (int i = 0; i < indent; i++) {
            spaces += " ";
        }
        std::cout << spaces + this->name << std::endl;
    }
};

std::string File::toString (){
    return "PATH = "+path+" NAME = "+name;
}

std::string File::getFatherFromPath(std::string path){
    return path.substr(0,path.find_last_of("/"));
}