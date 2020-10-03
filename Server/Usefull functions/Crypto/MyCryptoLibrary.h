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
    std::string digest;
    CryptoPP::SHA1 hash;
    std::ifstream input( filePath );
    if (input.is_open()) {
        std::string line;
        while (std::getline(input, line)) {
            // using printf() in all tests for consistency
            hash.Update(reinterpret_cast<const byte*>(line.data()), line.size());
        }
        input.close();
    }else{
        return std::string("DIGEST-ERROR");
    }
    digest.resize(hash.DigestSize());
    hash.Final(reinterpret_cast<byte*>(&digest[0]));
    std::string encoded;
    CryptoPP::StringSource(digest, true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded)));
    encoded = encoded.substr(0,encoded.size()-1); // remove "\n" at the end

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

void appendDigest(const std::string& str){
    hash_to_append.Update(reinterpret_cast<const byte*>(str.c_str()),str.size());
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