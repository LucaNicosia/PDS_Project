//
// Created by giuseppetoscano on 09/08/20.
//

#ifndef PDS_PROJECT_CLIENT_CRYPTO_H
#define PDS_PROJECT_CLIENT_CRYPTO_H

#include "cryptopp/hex.h"
#include "cryptopp/files.h"
#include "cryptopp/sha.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/filters.h"
#include "cryptopp/base64.h"

CryptoPP::SHA1 hash_to_append;

//Hash a file, which path is given.
std::string computeDigest(std::string filePath){
    //std::cout<<"computing "<<filePath<<std::endl;
    const int SIZE = 4096;
    std::string digest;
    CryptoPP::SHA1 hash;
    std::ifstream input( filePath,std::ios::binary);
    if (input.is_open()) {
        input.seekg(0,input.end);
        unsigned long long int length = input.tellg();
        //std::cout<<length<<std::endl;
        input.seekg(0,input.beg);
        char line[SIZE]="";
        //std::cout<<"prima"<<std::endl;
        unsigned long long int ri,rf;
        while (length > 0) {
            ri = input.tellg();
            input.read(line,sizeof(line));
            //std::cout<<line<<std::endl;
            rf = input.tellg();
            if(rf == static_cast<unsigned long long int>(-1)){
                rf = length;
                ri = 0;
            }
            length -= (rf - ri);
            //std::cout<<rf<<"-"<<ri<<" ";
            //std::cout<<rf - ri<<std::endl;
            hash.Update(reinterpret_cast<const byte*>(line), rf - ri);
        }
        //std::cout<<"dopo"<<std::endl;
        input.close();
    }else{
        return std::string("DIGEST-ERROR");
    }
    digest.resize(hash.DigestSize());
    hash.Final(reinterpret_cast<byte*>(&digest[0]));
    std::string encoded;
    CryptoPP::StringSource(digest, true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded)));
    encoded = encoded.substr(0,encoded.size()-1); // remove "\n" at the end
    input.close();
    //std::cout<<"ending "<<filePath<<std::endl;
    return encoded;
}

std::string computePasswordDigest(std::string saltedPassword){
    std::string digest;
    CryptoPP::SHA1 hash;
    hash.Update(reinterpret_cast<const byte*>(saltedPassword.data()), saltedPassword.size());
    digest.resize(hash.DigestSize());
    hash.Final(reinterpret_cast<byte*>(&digest[0]));
    std::string encoded;
    CryptoPP::StringSource(digest, true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded)));
    encoded = encoded.substr(0,encoded.size()-1); // remove "\n" at the end

    return encoded;
}

void appendDigest(const char* str, const int size){
    //std::cout<<"append: "<<str<<std::endl;
    hash_to_append.Update(reinterpret_cast<const byte*>(str),size);
}

std::string getAppendedDigest(){
    std::string digest,encoded;
    digest.resize(hash_to_append.DigestSize());
    hash_to_append.Final(reinterpret_cast<byte*>(&digest[0]));
    hash_to_append.Restart();
    CryptoPP::StringSource(digest, true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded)));
    encoded = encoded.substr(0,encoded.size()-1); // remove "\n" at the end
    return encoded;
}


bool compareDigests(std::string digest1, std::string digest2){
    return digest1.compare(digest2) == 0;
}

#endif //PDS_PROJECT_CLIENT_CRYPTO_H