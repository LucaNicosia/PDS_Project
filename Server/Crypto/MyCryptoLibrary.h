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
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <sstream>
/*
std::string b64_encode(std::string digest){
    using namespace boost::archive::iterators;

    std::stringstream os;
    typedef
    insert_linebreaks<         // insert line breaks every 72 characters
            base64_from_binary<    // convert binary values to base64 characters
                    transform_width<   // retrieve 6 bit integers from a sequence of 8 bit bytes
                            const char *,
                            6,
                            8
                    >
            >
            ,72
    >
            base64_text; // compose all the above operations in to a new iterator

    std::copy(
            base64_text(digest.c_str()),
            base64_text(digest.c_str() + digest.size()),
            ostream_iterator<char>(os)
    );

    return os.str();
} */

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
    /*std::cout << "Digest: ";
    CryptoPP::StringSource d = CryptoPP::StringSource(digest, true, new CryptoPP::Redirector(encoder));*/
    std::string encoded;
    CryptoPP::StringSource(digest, true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded)));
    encoded = encoded.substr(0,encoded.size()-1); // remove "\n" at the end
    std::cout <<"DIGEST: "<<encoded<< std::endl;

    return encoded;
}


bool compareDigests(std::string digest1, std::string digest2){
    int compare = digest1.compare(digest2);
    if (compare != 0){
        return false;
    }
    /*else if(compare == 0)
        std::cout << "Strings are equal"<<std::endl;*/
    return true;
}

#endif //PDS_PROJECT_CLIENT_CRYPTO_H