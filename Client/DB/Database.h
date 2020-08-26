//
// Created by root on 07/08/20.
//

#ifndef PDS_PROGETTO_DATABASE_H
#define PDS_PROGETTO_DATABASE_H

// database mySql libraries
#include <sqlite3.h>
#include <iostream>
#include <vector>

static int num_rec;

template<typename record_type>
static int callback(void* data, int argc, char **argv, char **azColName){
    int i;
    num_rec++;
    //record_type* elem = reinterpret_cast<record_type*>(data);
    std::vector<record_type>elem = &data;
    for(i = 0; i<argc; i++){
        std::cout<<"\t callback: "<<azColName[i]<<" = "<<argv[i]<<"\n";
        elem[num_rec].set(azColName[i],argv[i] ? argv[i] : NULL); // insert value in the right place
        std::cout<<"fatto\n";
    }
    return 0;
}

class Database {
    sqlite3 *db;
    std::string db_name;
    int status;

public:

    Database(std::string db_name):db_name(db_name),status(-1){}
    ~Database(){
        close();
    }

    int open() {
        int rc = 0;
        rc = sqlite3_open(db_name.c_str(), &db);


        if (rc < 0) {
            std::cerr << "Error open DB " << sqlite3_errmsg(db) << std::endl;
            return (-1);
        }
        else
            std::cout << "Opened Database "<<db_name<<" Successfully!" << std::endl;
        status = rc;
        return rc;
    }
    int close(){
        if(status == SQLITE_OK) {
            status = -1;
            sqlite3_close(db);
            std::cout<<"Database "<<db_name<<" closed"<<std::endl;
        }
        return 0;
    }

    int exec(std::string sql){
        std::cout<<"sql: "<<sql<<std::endl;
        char* zErrMsg;
        int rc = sqlite3_exec(db,sql.c_str(), nullptr, 0, &zErrMsg);
        if( rc != SQLITE_OK ) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        return rc;
    }

    template<typename record_type>
    int select(std::string sql, int &n_record, std::vector<record_type>& records){ // 'select' funtion needs a class that contains 'set' function
        n_record = 0;
        records.clear();
        sqlite3_stmt* stmt;
        int ret_code = 0;
        if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            printf("ERROR: while compiling sql: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            sqlite3_finalize(stmt);
            return -1;
        }
        std::string value;
        while((ret_code = sqlite3_step(stmt)) == SQLITE_ROW) {
            records.push_back(record_type());
            for(int i=0;i<sqlite3_data_count(stmt);i++){
                if(sqlite3_column_type(stmt,i) == SQLITE_NULL) {
                    value = "";
                }
                else {
                    value = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
                }
                records.back().set(sqlite3_column_name(stmt,i),value);
            }
            n_record++;
        }
        if(ret_code != SQLITE_DONE) {
            //this error handling could be done better, but it works
            printf("ERROR: while performing sql: %s\n", sqlite3_errmsg(db));
            printf("ret_code = %d\n", ret_code);
        }
        return ret_code;
    }
};


#endif //PDS_PROGETTO_DATABASE_H
