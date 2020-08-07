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
#include "TCP_Socket/Socket.h"


#define PORT 5058
#define MAXFD 50000
#define BUFFER_SIZE 1024

/*static int callback(void *data, int argc, char **argv, char **azColName){
    int i;
    std::cout<< reinterpret_cast<const char*>(data)<<std::endl;

    for(i = 0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");
    return 0;
}*/

int main(int argc, char** argv)
{
    /*sqlite3* db;
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
    //sql = "SELECT * from DIRECTORY";

    /* Execute SQL statement */
    /*rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);

    if( rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Operation done successfully\n");
    }
    sqlite3_close(db);*/

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
