//
// Created by root on 07/08/20.
//

#include "Database.h"

/*
int num_rec;

template<typename record_type>
template<> int Database<record_type>::DB_open(){
    int rc = 0;
    rc = sqlite3_open(db_name.c_str(), &db);


    if (rc) {
        std::cerr << "Error open DB " << sqlite3_errmsg(db) << std::endl;
        return (-1);
    }
    else
        std::cout << "Opened Database Successfully!" << std::endl;
    status = rc;
    return rc;
}

template<typename record_type>
int Database<record_type>::DB_close(){
    if(status == SQLITE_OK)
        sqlite3_close(db);
    return 0;
}

template<typename record_type>
static int callback(void *data, int argc, char **argv, char **azColName){
    int i;
    record_type* elem = reinterpret_cast<record_type*>(data);

    for(i = 0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        elem[num_rec].azColName[i] = argv[i] ? argv[i] : NULL; // insert value in the right place
    }
    num_rec++;

    printf("\n");
    return 0;
}

template<typename record_type>
int Database<record_type>::DB_query(const char *sql, int &n_record, record_type records[]){
    char* zErrMsg;
    num_rec = 0;
    int rc = sqlite3_exec(db, sql, callback<record_type>,(void*) records, &zErrMsg);
    num_rec = 0;

    if( rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Operation done successfully\n");
    }
    return rc;
}
*/