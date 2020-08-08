//
// Created by giuse on 27/04/2020.
//

#ifndef LAB02_FILE_H
#define LAB02_FILE_H

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include "Directory.h"


class Directory;

class File {

    uintmax_t size;

public:

    //Costruttore
    File (std::string name, uintmax_t size, std::weak_ptr<Directory> dFather);

    virtual int mType () const;
    virtual void ls (int indent) const;
};


#endif //LAB02_FILE_H
