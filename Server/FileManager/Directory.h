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

public:
    //Costruttore
    Directory();

    ~Directory();

    std::shared_ptr<Directory> addDirectory(std::string dName);
    static std::shared_ptr<Directory> makeDirectory( std::string dName, std::weak_ptr<Directory> dFather);
    std::shared_ptr<File> addFile (const std::string name, const std::string &hash);
    bool renameDir (const std::string& oldName, const std::string& newName);
    bool renameFile (const std::string& oldName, const std::string& newName);
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

    const std::weak_ptr<Directory> &getSelf() const;

    static std::string getFatherFromPath(std::string path);
    static std::shared_ptr<Directory> getRoot();
    void ls(int indent) const;
};

static std::shared_ptr<Directory> root;

#endif
