//
// Created by giuseppetoscano on 09/08/20.
//

#include "cryptopp/hex.h"
#include "cryptopp/files.h"
#include "cryptopp/sha.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/filters.h"

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
        return std::string("SYNC-ERROR");
    }
    digest.resize(hash.DigestSize());
    hash.Final((byte*)&digest[0]);
    /*std::cout << "Digest: ";
    CryptoPP::StringSource(digest, true, new CryptoPP::Redirector(encoder));
    std::cout << std::endl;*/

    return digest;
}


bool compareDigests(std::string digest1, std::string digest2){
    int compare = digest1.compare(digest2);
    if (compare != 0){
        std::cout << digest1 << " is not equal to "<< digest2 << std::endl;
        return false;
    }
    else if(compare == 0)
        std::cout << "Strings are equal"<<std::endl;
    return true;
}
