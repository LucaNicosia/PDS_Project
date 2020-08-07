//
// Created by giuse on 27/04/2020.
//

#ifndef LAB02_DIRECTORY_H
#define LAB02_DIRECTORY_H

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include "File.h"
#include "Base.h"
#include <typeinfo>

class File;

class Directory: public Base {

    std::weak_ptr<Directory> dFather;
    std::weak_ptr<Directory> self;
    std::vector <std::shared_ptr<Directory>> dSons;
    std::vector<std::shared_ptr<File>> fSons;
    std::string path;

public:
    //Costruttore
    Directory();

    ~Directory();

    std::string toString();

    //void ls(int indent);
    virtual void ls (int indent) const;

    //template <typename T>
    //std::shared_ptr<Directory> get(std::string name);
    std::shared_ptr<Base> get (const std::string& name);
    //std::shared_ptr<Directory> addSelf();

    std::shared_ptr<Directory> addDirectory(std::string dName);

    static std::shared_ptr<Directory> makeDirectory(std::string dName, std::weak_ptr<Directory> dFather);

    static std::shared_ptr<Directory> getRoot();

    std::shared_ptr<File> addFile (const std::string& nome, uintmax_t size);

    bool remove (const std::string& nome);

    virtual int mType () const;

    std::shared_ptr<Directory> getDir (const std::string& name);

    std::shared_ptr<File> getFile (const std::string& name);
};

static std::shared_ptr<Directory> root;


#endif //LAB02_DIRECTORY_H
