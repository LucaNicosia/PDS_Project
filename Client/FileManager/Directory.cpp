//
// Created by giuse on 27/04/2020.
//

#include "Directory.h"
#include <typeinfo>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

Directory::Directory():Base() {
    //std::cout<<"Costruttore vuoto @ "<<this<<std::endl;
}

Directory::~Directory() {
    //std::cout<<"Distruttore vuoto @ "<<this<<std::endl;
}


void Directory::ls(int indent) const{
    std::string spaces;

    if (this != 0) {

        for (int i = 0; i < indent; i++) {
            spaces += " ";
        }
        std::cout << spaces + this->name << std::endl;

        for (int i = 0; i < dSons.size(); i++) {
            std::cout << spaces + "   " + this->dSons[i]->name << std::endl;
        }
        for (int i = 0; i < fSons.size(); i++) {
            //std::cout<<"Dentro ls file "<<this->fSons[i]<<" fileName = "<<this->fSons[i]->getName()<<std::endl;
            std::cout << spaces + "    " + this->fSons[i]->getName() << std::endl;
        }
    }

}

    std::shared_ptr<Base> Directory::get (const std::string& name){
        if (name == "..")
            return std::dynamic_pointer_cast<Base>(std::shared_ptr<Directory>(root));


        if (name == ".")
            return std::dynamic_pointer_cast<Base>( std::shared_ptr<Directory>(self));

        for (int i = 0; i < fSons.size(); i++){
            if (fSons[i]->getName() == name){
                return std::dynamic_pointer_cast<Base>( std::shared_ptr<File>(fSons[i]));
            }
        }

        for (int i = 0; i < dSons.size(); i++){
            if (dSons[i]->name == name){
                return std::dynamic_pointer_cast<Base>( std::shared_ptr<Directory>(dSons[i]));
            }
        }

        return nullptr;
        }

    std::shared_ptr<Directory> Directory::addDirectory(std::string dName){

        if (this != 0){
            std::shared_ptr<Directory> newDir = makeDirectory(dName, self);
            dSons.push_back(newDir);
            fs::create_directories(newDir->path);
            return newDir;
        }else
            return nullptr;
    }

    std::shared_ptr<Directory> Directory::makeDirectory(std::string dName, std::weak_ptr<Directory> dFather){
        std::shared_ptr<Directory> newDir = std::make_shared<Directory>();
        newDir->name = dName;
        newDir->dFather = dFather;
        newDir->self = newDir;
        newDir->dSons = std::vector<std::shared_ptr<Directory>>();
        newDir->fSons = std::vector<std::shared_ptr<File>>();
        if (dName != "stocazzo")
            newDir->path = newDir->dFather.lock()->path+"/"+dName;
        else
            newDir->path = "stocazzo";
        return newDir;
    }

    std::shared_ptr<Directory> Directory::getRoot(){

        if (root == std::shared_ptr<Directory>()) {
                std::cout << "Entro" << std::endl;
                root = makeDirectory("stocazzo", std::weak_ptr<Directory>());
        }
        return root;
    }

    std::string Directory::toString(){
        return "Il nome della cartella è "+name;
    }

    std::shared_ptr<File> Directory::addFile (const std::string& name, uintmax_t size){

        if (this != 0){

            //File file {name, size, this->self};
            //std::shared_ptr<File> tmp (&file);
            std::shared_ptr<File> file = std::make_shared<File>(name, size, this->self);
            fSons.push_back(file);
            file->path = file->dFather.lock()->path+"/"+name;
            std::ofstream(file->path);
            //std::cout<<"Dentro add file "<<this->fSons[0]<<" fileName = "<<this->fSons[0]->getName()<<std::endl;
            return file;
        }else
            return nullptr;
    }

    bool Directory::remove (const std::string& name){
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

        for (int i = 0; i < fSons.size(); i++){
            if (name == fSons[i]->getName()){
                fSons.erase(fSons.begin()+i);
                return true;
            }
        }

        return false;
    }

    int Directory::mType () const {
        return DIRECTORY_TYPE;
    }

std::shared_ptr<Directory> Directory::getDir (const std::string& name){
        return std::dynamic_pointer_cast<Directory>(get(name)) ;
    }

std::shared_ptr<File> Directory::getFile (const std::string& name){
    return std::dynamic_pointer_cast<File>(get(name)) ;
}