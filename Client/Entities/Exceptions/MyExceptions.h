//
// Created by root on 04/10/20.
//

#ifndef PDS_PROJECT_CLIENT_MYEXCEPTIONS_H
#define PDS_PROJECT_CLIENT_MYEXCEPTIONS_H

#include <iostream>
#include <exception>

class general_exception : public std::exception{
protected:
    std::string error_msg;
public:
    general_exception(const std::string &errorMsg) : error_msg(errorMsg){}

    const char* what() const throw() {
        return error_msg.c_str();
    }
};

class socket_exception : public general_exception{
public:
    socket_exception(const std::string &errorMsg) : general_exception(errorMsg){}
};

class filesystem_exception : public general_exception{
public:
    filesystem_exception(const std::string &errorMsg) : general_exception(errorMsg){}
};

class database_exception : public general_exception{
public:
    database_exception(const std::string &errorMsg) : general_exception(errorMsg){}
};


#endif //PDS_PROJECT_CLIENT_MYEXCEPTIONS_H
