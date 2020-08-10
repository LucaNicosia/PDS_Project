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

// Communication
#include "Communication/Communication.cpp"

#include "FileManager/Directory.h"
#include "FileManager/File.h"
#include "FileManager/FileWatcher.h"



#define PORT 5072
#define MAXFD 50000

auto modification_function = [](std::string file, FileStatus fs, FileType ft){
    std::cout<<file;
    if(ft == FileType::file)
        std::cout<<" file";
    else
        std::cout<<" directory";
    switch (fs) {
        case FileStatus::created:
            std::cout<<" created\n";
            break;
        case FileStatus::erased:
            std::cout<<" erased\n";
            break;
        case FileStatus::modified:
            std::cout<<" modified\n";
            break;
    }
};

int main(int argc, char** argv)
{
    Socket s{};
    /*struct sockaddr_in addr;
    unsigned int len = sizeof(addr);


    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr)<=0){
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    s.connect(&addr, len);*/

    s.inizialize_and_connect(PORT, AF_INET, "127.0.0.1");
    std::string msg = syncRequest(s, "ciao");
    if (msg == "SYNC-ERROR"){
        std::cout<<"SYNC-ERROR"<<std::endl;
    }else{
        if (compareDigests(computeDigest("./DB/ciao.txt"), msg)){
            //SAME DIGEST
            std::cout<<"SAME DIGEST"<<std::endl;
            sendMsg(s, "DONE");
        }else{
            //DIFFERENT DIGEST
            std::cout<<"DIFFERENT DIGEST"<<std::endl;
            //--------------
            //---checkDB----
            //--------------
            //AT THE END
            sendMsg(s, "DONE");
        }
    }

/*
    FileWatcher FW("./TestPath/",std::chrono::milliseconds(5000));
    Socket s;

    s.inizialize_and_connect(PORT,AF_INET,"127.0.0.1");

    // SYN with server completed, starting to monitor client directory
    FW.start(modification_function);

    /*
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
        std::cout<<"directory["<<i<<"]-> id: "<<records[i].getId()<<" path: "<<records[i].getPath()<<std::endl;
    }
    for(int i=0;i<n_files;i++){
        std::cout<<"files["<<i<<"]-> id: "<<files[i].getId()<<" id_dir: "<<files[i].getIdDir()<<" nome: "<<files[i].getName()<<" hash: "<<files[i].getHash()<<std::endl;
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

    s.syncRequest("ciao");
    s.compareDBDigest("./DB/ciao.txt");

    s.sendDir("./client_directory/prova");
    std::cout<<s.rcvMsg()<<std::endl;

    s.sendFile("./client_directory/file.txt");
    std::cout<<s.rcvMsg()<<std::endl;
    */

    return 0;
}

