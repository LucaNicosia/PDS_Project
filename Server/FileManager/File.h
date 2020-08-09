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
    int id;
    int id_dir;
    std::string name;
    std::string digest;
    std::weak_ptr<Directory> dFather;

public:

    //Costruttore
    File ();
    File (std::string &name, int id, int id_dir, std::string &digest, std::weak_ptr<Directory> dFather);

    int getId() const;

    void setId(int id);

    int getIdDir() const;

    void setIdDir(int idDir);

    const std::string &getName() const;

    void setName(const std::string &name);

    const std::string &getDigest() const;

    void setDigest(const std::string &digest);

    const std::weak_ptr<Directory> &getDFather() const;

    void setDFather(const std::weak_ptr<Directory> &dFather);

    void set(std::string field, std::string value);

};


#endif //LAB02_FILE_H
