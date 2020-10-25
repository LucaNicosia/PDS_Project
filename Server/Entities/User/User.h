#ifndef USER_H
#define USER_H

#include <string>
#include <iostream>
#include "../../Usefull functions/Crypto/MyCryptoLibrary.h"

#define SIZE_SALT 10

class User {
    std::string username;
    std::string password;
    std::string salt;

public:

    User(){}
    User(std::string username, std::string password):username(username){
        this->salt = "";
        for (int i = 0; i < SIZE_SALT; i++){
            salt += "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"[random () % 52];
        }

        this->password = computePasswordDigest(password+salt);
    }
    ~User(){}

    std::string getUsername() const{
        return username;
    }

    std::string getPassword() const{
        return password;
    }

    std::string getSalt() const{
        return salt;
    }

    std::string toString(){
        return "USERNAME = "+username+" PASSWORD = "+password+" SALT = "+salt;
    }

    void set(std::string field, std::string value){

        if(field == "username"){
            username = value;
        }else if(field == "password"){
            password = value;
        }else if(field == "salt"){
            salt = value;
        }else{
            throw std::runtime_error("Directory: Invalid field! ("+field+")");
        }
    }
};

#endif //USER_H
