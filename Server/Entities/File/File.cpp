#include "File.h"

File::File() {
    path = "";
    hash = "";
    name = "";
}

File::File(const std::string name, const std::string &hash, std::weak_ptr<Directory> dFather) {
    this->name = name;
    std::string path = dFather.lock()->getPath()+"/"+name;
    if(path.find_first_of("/") == 0)
        this->path = path.substr(1);
    else
        this->path = path;
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

void File::setName(const std::string &name) {
    File::name = name;
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
    }else if(field == "name"){
        name = value;
    }else{
        throw general_exception("File: invalid field");
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
    return "PATH = "+path+" NAME = "+name+" HASH = "+hash;
}

std::string File::getFatherFromPath(std::string path){
    return path.substr(0,path.find_last_of("/"));
}