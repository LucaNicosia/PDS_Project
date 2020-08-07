//
// Created by giuse on 27/04/2020.
//

#include "File.h"


//Costruttore
File::File (std::string name, uintmax_t size, std::weak_ptr<Directory> dFather):size(size), dFather(dFather), Base(){
    this->name = name;
}

int File::mType () const {
    return FILE_TYPE;
}

void File::ls (int indent) const{

    std::string spaces;

    if (this != 0) {

        for (int i = 0; i < indent; i++) {
            spaces += " ";
        }
        std::cout << spaces + this->name <<" "<<this->size<< std::endl;
    }
};