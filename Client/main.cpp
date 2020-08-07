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

int main(int argc, char** argv)
{
    Database<DIR> DB("../DB/user.db");
    DIR records[10];
    int n_rec;
    DB.DB_open();
    DB.DB_query("SELECT * FROM DIRECTORY",n_rec,records);
    DB.DB_close();

    std::cout<<n_rec<<"\n";
    for(int i=0;i<n_rec;i++){
        std::cout<<"record["<<i<<"]-> id: "<<records[i].id<<" path: "<<records[i].path<<std::endl;
    }

    return (0);
}
