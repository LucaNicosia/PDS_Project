#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <chrono>
#include <sstream>
#include <mutex>

class Logger{
    std::mutex m;
    bool use_mutex;
    std::string log_file_path;

    std::ostringstream getTimestamp(){
        time_t now = time(0);
        tm *ltm = localtime(&now);
        std::ostringstream timestamp;

        // timestamp format: YEAR MONTH DAY HOUR:MINUTE:SECOND
        std::string of1 = (1 + ltm->tm_mon < 10)?"0":"";
        std::string of2 = (1 + ltm->tm_mday < 10)?"0":"";
        std::string of3 = (ltm->tm_hour < 10)?"0":"";
        std::string of4 = (ltm->tm_min < 10)?"0":"";
        std::string of5 = (ltm->tm_sec < 10)?"0":"";
        timestamp << 1900 + ltm->tm_year <<of1<< 1 + ltm->tm_mon <<of2<<  ltm->tm_mday<<" "<<of3<< ltm->tm_hour << ":"<<of4<<ltm->tm_min << ":"<<of5<<ltm->tm_sec;
        return std::move(timestamp);
    }


public:
    Logger():log_file_path(""),use_mutex(false){};

    void setUseMutex(bool useMutex) {
        use_mutex = useMutex;
    }

    void setLogFilePath(const std::string &logFilePath) {
        if(logFilePath == "")
            throw std::runtime_error("empty log file path");
        log_file_path = logFilePath;
    }

    const std::mutex &getM() const {
        return m;
    }

    bool useMutex() const {
        return use_mutex;
    }

    const std::string &getLogFilePath() const {
        return log_file_path;
    }

    void writeLog(const std::ostringstream& str){
        if(str.str() == "")
            return;
        std::ofstream ofs;
        ofs.open (log_file_path, std::ofstream::out | std::ofstream::app);
        if(use_mutex) {
            std::lock_guard<std::mutex> lg(m);
            ofs << getTimestamp().str() << " " << std::this_thread::get_id() << " " << str.str()<<std::endl; // TIMESTAMP tid <message>
        }
        else{
            ofs << getTimestamp().str() << " " << std::this_thread::get_id() << " " << str.str()<<std::endl; // TIMESTAMP tid <message>
        }
    }

    void writeLogAndClear(std::ostringstream& str){
        writeLog(str);
        str.str("");
    }
};

static Logger Log_Writer;


#endif //PDS_PROJECT_SERVER_UTILITIES_H
