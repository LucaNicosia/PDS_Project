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
    std::string path;
    std::string hash;
    std::weak_ptr<Directory> dFather;

public:

    //Costruttore
    File ();
    File (const std::string path, int id, int id_dir, const std::string &hash, std::weak_ptr<Directory> dFather);

    int getId() const;

    void setId(int id);

    int getIdDir() const;

    void setIdDir(int idDir);

    const std::string &getPath() const;

    void setPath(const std::string &path);

    const std::string &getHash() const;

    void setHash(const std::string &hash);

    const std::weak_ptr<Directory> &getDFather() const;

    void setDFather(const std::weak_ptr<Directory> &dFather);

    void set(std::string field, std::string value);

    std::string getFatherPath();
};


#endif //LAB02_FILE_H
