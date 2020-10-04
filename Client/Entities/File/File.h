//
// Created by giuse on 27/04/2020.
//

#ifndef LAB02_FILE_H
#define LAB02_FILE_H

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include "../Directory/Directory.h"
#include "../Exceptions/MyExceptions.h"


class Directory;

class File {
    std::string path;
    std::string hash;
    std::weak_ptr<Directory> dFather;
    std::string name;

public:

    //Costruttore
    File ();
    File (const std::string path, const std::string &hash, std::weak_ptr<Directory> dFather);
    File (const File& other);
    File& operator=(const File& in);

    const std::string &getPath() const;

    void setPath(const std::string &path);

    const std::string &getHash() const;

    void setHash(const std::string &hash);

    void setName(const std::string &name);

    const std::string &getName() const;

    const std::weak_ptr<Directory> &getDFather() const;

    void setDFather(const std::weak_ptr<Directory> &dFather);

    void set(const std::string& field, const std::string& value);

    std::string getFatherPath();

    std::string toString();

    void ls (int indent) const;
    static std::string getFatherFromPath(std::string path);
};


#endif //LAB02_FILE_H
