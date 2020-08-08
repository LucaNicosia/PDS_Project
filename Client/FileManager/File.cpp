//
// Created by giuse on 27/04/2020.
//

#include "File.h"


//Costruttore

File::File() {}

File::File(std::string &name, int id, int id_dir, std::string &hash, std::weak_ptr<Directory> dFather) {
    this->name = name;
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

const std::string &File::getName() const {
    return name;
}

void File::setName(const std::string &name) {
    File::name = name;
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
        name = value;
    }else if(field == "hash"){
        hash = value;
    }else{
        std::cout<<"Invalid field!\n"; // QUI CI VUOLE UNA ECCEZIONE
    }
}
