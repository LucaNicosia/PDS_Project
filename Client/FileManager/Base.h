//
// Created by giuse on 28/04/2020.
//

#ifndef LAB02_BASE_H
#define LAB02_BASE_H


#include <string>

#define DIRECTORY_TYPE 1
#define FILE_TYPE 2

class Base {
protected:
    std::string name;
public:
    // Methods
    std::string getName () const;
    virtual int mType () const = 0;
    virtual void ls (int indent) const = 0;
};


#endif //LAB02_BASE_H
