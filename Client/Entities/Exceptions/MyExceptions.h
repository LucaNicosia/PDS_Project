//
// Created by root on 04/10/20.
//

#ifndef PDS_PROJECT_CLIENT_MYEXCEPTIONS_H
#define PDS_PROJECT_CLIENT_MYEXCEPTIONS_H

#include <iostream>
#include <exception>

class socket_exception : public std::exception{
private:
    std::string error_msg;
public:
    socket_exception(const std::string &errorMsg) : error_msg(errorMsg){}

    const char* what() const throw() {
        return error_msg.c_str();
    }
};

class filesystem_exception : public std::exception{
private:
    std::string error_msg;
public:
    filesystem_exception(const std::string &errorMsg) : error_msg(errorMsg){}

    const char* what() const throw() {
        return error_msg.c_str();
    }
};

class general_exception : public std::exception{
private:
    std::string error_msg;
public:
    general_exception(const std::string &errorMsg) : error_msg(errorMsg){}

    const char* what() const throw() {
        return error_msg.c_str();
    }
};

class database_exception : public std::exception{
private:
    std::string error_msg;
public:
    database_exception(const std::string &errorMsg) : error_msg(errorMsg){}

    const char* what() const throw() {
        return error_msg.c_str();
    }
};


#endif //PDS_PROJECT_CLIENT_MYEXCEPTIONS_H
