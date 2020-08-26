//
//
//

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <string>
#include <iostream>
#include <vector>
#include <memory>

#include <typeinfo>

class File;

class Directory {

    std::weak_ptr<Directory> dFather;
    std::weak_ptr<Directory> self;
    std::vector <std::shared_ptr<Directory>> dSons;
    std::vector<std::shared_ptr<File>> fSons;
    std::string path;
    std::string name;

    //ROOT
    std::weak_ptr<Directory> root;

public:
    //Costruttore
    Directory();

    ~Directory();

    std::shared_ptr<Directory> addDirectory(std::string dName, const bool& create_flag);
    static std::shared_ptr<Directory> makeDirectory( std::string dName, std::weak_ptr<Directory> dFather);
    std::shared_ptr<File> addFile (const std::string name, const std::string &hash, const bool& create_flag);
    bool removeDir (const std::string& nome);
    bool removeFile (const std::string& nome);
    std::shared_ptr<Directory> getDir (const std::string& name);
    std::shared_ptr<File> getFile (const std::string& name);

    void set(std::string field, std::string value);

    const std::string &getPath() const;

    void setPath(const std::string &path);

    const std::string &getName() const;

    void setName(const std::string &name);

    const std::weak_ptr<Directory> &getDFather() const;
    const std::vector <std::shared_ptr<Directory>> &getDSons() const;
    const std::vector <std::shared_ptr<File>> &getFSons() const;
    const std::weak_ptr<Directory> &getSelf() const;

    static std::string getFatherFromPath(std::string path);
    void ls(int indent) const;

    std::weak_ptr<Directory> getRoot();
    std::weak_ptr<Directory> setRoot(std::weak_ptr<Directory> root);

    std::string toString ();
};

#endif
