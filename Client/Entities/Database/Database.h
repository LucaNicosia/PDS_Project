#ifndef DATABASE_H
#define DATABASE_H

// database mySql libraries
#include <sqlite3.h>
#include <iostream>
#include <vector>
#include <fstream>
#include "../../Entities/Exceptions/MyExceptions.h"


class Database {
    sqlite3 *db;
    sqlite3_stmt* stmt;
    std::string db_name;
    int status;

public:

    Database(std::string db_name):db_name(db_name),status(-1),stmt(nullptr){}
    ~Database(){
        close();
    }

    int open() {
        std::ifstream db_file(db_name);
        if(!db_file){
            // a file that doesn't exits
            return -1;
        }

        int rc;
        rc = sqlite3_open(db_name.c_str(), &db);


        if (rc) {
            throw filesystem_exception("database: "+std::string(sqlite3_errmsg(db)));
        }
        else
            if (DEBUG)
                std::cout << "Opened Database "<<db_name<<" Successfully!" << std::endl;
        status = rc;
        return rc;
    }
    int close(){
        if(status == SQLITE_OK) {
            status = -1;
            if(stmt)
                sqlite3_finalize(stmt);
            stmt = nullptr;
            sqlite3_close(db);

            if (DEBUG)
            std::cout<<"Database "<<db_name<<" closed"<<std::endl;
        }
        return 0;
    }

    int exec(std::string sql){
        char* zErrMsg;
        int rc = sqlite3_exec(db,sql.c_str(), nullptr, 0, &zErrMsg);
        if( rc != SQLITE_OK ) {
            std::string sErr = zErrMsg;
            sqlite3_free(zErrMsg);
            throw database_exception("SQL error: %s\n"+sErr);
        }
        return rc;
    }

    template<typename record_type>
    int select(std::string sql, int &n_record, std::vector<record_type>& records){ // 'select' funtion needs a class that contains 'set' function
        n_record = 0;
        records.clear();
        int ret_code = 0;
        if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            std::string sErr = sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            throw database_exception("ERROR: while compiling sql: "+sErr);
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
                records.back().set(sqlite3_column_name(stmt,i),value); // set function is needed in objects that uses this class
            }
            n_record++;
        }
        if(ret_code != SQLITE_DONE) {
            std::string sErr = sqlite3_errmsg(db);
            throw database_exception("ERROR: while performing sql: "+sErr);
        }
        return ret_code;
    }
};


#endif //DATABASE_H
