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
#include "TCP_Socket/Socket.h"

#include "./FileManager/File.h"
#include "./FileManager/Directory.h"

#define PORT 5058
#define MAXFD 50000
#define BUFFER_SIZE 1000

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
    Directory records[10];
    File files[10];
    int n_rec,n_files;
    DB.DB_open();
    DB.DB_query("SELECT * FROM DIRECTORY",n_rec,records);
    DB.DB_query("SELECT * FROM FILE",n_files,files);
    DB.DB_close();

    printf("\n");
    std::cout<<n_rec<<"\n";
    for(int i=0;i<n_rec;i++){
        std::cout<<"directory["<<i<<"]-> id: "<<records[i].getId()<<" path: "<<records[i].getPath()<<" name: "<<records[i].getName()<<std::endl;
    }
    for(int i=0;i<n_files;i++){
        std::cout<<"files["<<i<<"]-> id: "<<files[i].getId()<<" id_dir: "<<files[i].getIdDir()<<" nome: "<<files[i].getName()<<" hash: "<<files[i].getHash()<<std::endl;
    }
    /*
    Socket s{};
    struct sockaddr_in addr;
    unsigned int len = sizeof(addr);


    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr)<=0){
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    s.connect(&addr, len);

    const char *buffer = "prova";
    s.write(buffer, BUFFER_SIZE, 0);

    s.sendFile("./client_directory/file.txt");
    */

    return 0;
}

