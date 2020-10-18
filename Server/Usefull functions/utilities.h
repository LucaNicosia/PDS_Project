//
// Created by root on 11/10/20.
//

#ifndef PDS_PROJECT_SERVER_UTILITIES_H
#define PDS_PROJECT_SERVER_UTILITIES_H

#include <iostream>
#include <chrono>
#include <sstream>
#include <mutex>


std::ostringstream getTimestamp();
void writeLog(const std::ostringstream& str, const std::string& log_path, std::mutex& m);
void writeLogAndClear(std::ostringstream& str, const std::string& log_path, std::mutex& m);

std::ostringstream getTimestamp(){
    time_t now = time(0);
    tm *ltm = localtime(&now);
    std::ostringstream timestamp;

    // timestamp format: YEAR MONTH DAY HOUR:MINUTE:SECOND
    std::string of1 = (1 + ltm->tm_mon < 10)?"0":"";
    std::string of2 = (1 + ltm->tm_mday < 10)?"0":"";
    std::string of3 = (1 + ltm->tm_hour < 10)?"0":"";
    std::string of4 = (1 + ltm->tm_min < 10)?"0":"";
    std::string of5 = (1 + ltm->tm_sec < 10)?"0":"";
    timestamp << 1900 + ltm->tm_year <<of1<< 1 + ltm->tm_mon <<of2<<  ltm->tm_mday<<" "<<of3<< 1 + ltm->tm_hour << ":"<<of4<<1 + ltm->tm_min << ":"<<of5<<1 + ltm->tm_sec;
    return std::move(timestamp);
}

void writeLog(const std::ostringstream& str, const std::string& log_path, std::mutex& m){
    std::lock_guard<std::mutex> lg(m);
    if(str.str() == "")
        return;
    std::ofstream ofs;
    ofs.open (log_path, std::ofstream::out | std::ofstream::app);
    ofs << getTimestamp().str() << " " << std::this_thread::get_id() << " " << str.str(); // TIMESTAMP tid <message>
}

void writeLogAndClear(std::ostringstream& str, const std::string& log_path, std::mutex& m){
    writeLog(str,log_path,m);
    str.str("");
}

#endif //PDS_PROJECT_SERVER_UTILITIES_H
