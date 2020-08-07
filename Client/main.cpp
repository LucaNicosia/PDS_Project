/*
 * Client
 * 
 */

#include <iostream>
#include <map>
#include <mutex>
#include <atomic>
#include <vector>
#include <thread>

// database mySql libraries
#include <sqlite3.h>
#include "./DB/Database.h"


#define PORT 5058
#define MAXFD 50000

class DIR{
public:
    int id;
    std::string path;

    void set(std::string ind, std::string value){
        if(ind == "id"){
            id = std::atoi(value.c_str());
        }else{
            path = value;
        }
    }
};

class _FILE{
public:
    int id;
    int id_dir;
    std::string nome;
    std::string hash;

    void set(std::string ind, std::string value){
        if(ind == "id"){
            id = std::atoi(value.c_str());
        }else if(ind == "id_dir"){
            id_dir = std::atoi(value.c_str());
        }else if(ind == "nome"){
            nome = value;
        }else if(ind == "hash"){
            hash = value;
        }
    }
};

int main(int argc, char** argv)
{
    Database DB("../DB/user.db");
    DIR records[10];
    _FILE files[10];
    int n_rec,n_files;
    DB.DB_open();
    DB.DB_query("SELECT * FROM DIRECTORY",n_rec,records);
    DB.DB_query("SELECT * FROM FILE",n_files,files);
    DB.DB_close();

    std::cout<<n_rec<<"\n";
    for(int i=0;i<n_rec;i++){
        std::cout<<"directory["<<i<<"]-> id: "<<records[i].id<<" path: "<<records[i].path<<std::endl;
    }
    for(int i=0;i<n_files;i++){
        std::cout<<"files["<<i<<"]-> id: "<<files[i].id<<" id_dir: "<<files[i].id_dir<<" nome: "<<files[i].nome<<" hash: "<<files[i].hash<<std::endl;
    }

    return (0);
}
