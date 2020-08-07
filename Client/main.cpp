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

// Socket
#include "./TCP_Socket/Socket.h"

#define PORT 5058
#define MAXFD 50000
#define BUFFER_SIZE 1000

static int callback(void *data, int argc, char **argv, char **azColName){
    int i;
    std::cout<< reinterpret_cast<const char*>(data)<<std::endl;

    for(i = 0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");
    return 0;
}

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
    sqlite3* db;
    int rc = 0;
    rc = sqlite3_open("../DB/user.db", &db);


    if (rc) {
        std::cerr << "Error open DB " << sqlite3_errmsg(db) << std::endl;
        return (-1);
    }
    else
        std::cout << "Opened Database Successfully!" << std::endl;
    char *sql;
    char *zErrMsg = 0;
    const char* data = "Callback function called";

    /* Create SQL statement */
    sql = "SELECT * from DIRECTORY";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);

    if( rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Operation done successfully\n");
    }
    sqlite3_close(db);

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

    return (0);
}
