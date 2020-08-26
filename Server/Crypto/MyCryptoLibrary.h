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

//Hash a file, which path is given.
std::string computeDigest(std::string filePath){
    CryptoPP::HexEncoder encoder(new CryptoPP::FileSink(std::cout));
    std::string digest;
    CryptoPP::SHA1 hash;
    std::ifstream input( filePath );
    if (input.is_open()) {
        std::string line;
        while (std::getline(input, line)) {
            // using printf() in all tests for consistency
            //std::cout<<line<<std::endl;
            hash.Update((const byte*)line.data(), line.size());
        }
        input.close();
    }else{
        return std::string("DIGEST-ERROR");
    }
    digest.resize(hash.DigestSize());
    hash.Final((byte*)&digest[0]);
    std::string encoded;
    CryptoPP::StringSource(digest, true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded)));
    encoded = encoded.substr(0,encoded.size()-1); // remove "\n" at the end

    return encoded;
}


bool compareDigests(std::string digest1, std::string digest2){
    return (digest1.compare(digest2)!=0)?false:true;
}

#endif //PDS_PROJECT_CLIENT_CRYPTO_H