//
// Created by root on 09/08/20.
//

#ifndef PDS_PROJECT_CLIENT_FILEWATCHER_H
#define PDS_PROJECT_CLIENT_FILEWATCHER_H

#include <filesystem>
#include <chrono>
#include <thread>
#include <map>
#include <string>
#include <functional>

// Define available file changes
enum class FileStatus {created, modified, erased, none};
enum class FileType {file, directory};

class FileWatcher {
    class FileStruct{
    public:
        std::filesystem::file_time_type last_mod;
        FileType type;

        FileStruct(std::filesystem::file_time_type last_mod, FileType type): last_mod(last_mod), type(type) {}
        FileStruct(){};
    };
    std::string path_to_watch;
    std::chrono::duration<int, std::milli> delay;
    std::map<std::string, FileStruct> paths;
    bool running;

    // Check if "paths" contains a given key
    bool contains(const std::string &key) {
        auto el = paths.find(key);
        return el != paths.end();
    }

public:

    FileWatcher(std::string path_to_watch, std::chrono::duration<int, std::milli> delay) : running(true), path_to_watch{path_to_watch}, delay{delay} {
        for(auto &file : std::filesystem::recursive_directory_iterator(path_to_watch)) {
            paths[file.path().string()] = FileStruct(std::filesystem::last_write_time(file),(file.is_directory())?FileType::directory:FileType::file);
        }
    }

    ~FileWatcher(){
        this->stop();
    }

    void start(const std::function<void (std::string, std::string, FileStatus, FileType)> &action) {
        struct tmp {
            std::string file;
            std::string filePath;
            FileStatus fs;
            FileType ft;
        };
        std::vector<struct tmp> t;
        while(running) {
            // Wait for "delay" milliseconds
            std::this_thread::sleep_for(delay);
            if(!running) return; // if when i wake up 'running' is 'false', don't do the 'action' function
            auto it = paths.begin();
            // check if a file was deleted
            while (it != paths.end() ) {
                //send files
                //std::ifstream file (it->first);
                if (!std::filesystem::exists(std::filesystem::status(it->first))){
                    if (paths[it->first].type == FileType::file) {

                        std::cout<<"F it->first = "<<it->first<<std::endl;
                        action(it->first, it->first, FileStatus::erased, it->second.type); // mando se è file
                        it = paths.erase(it);
                    }
                    else {
                        //preparing directories to send at the end
                        std::cout<<"D it->first = "<<it->first<<std::endl;
                        struct tmp x;
                        x.file = it->first;
                        x.filePath = it->first;
                        x.fs = FileStatus::erased;
                        x.ft = it->second.type;
                        t.push_back(x);
                        it = paths.erase(it);
                    }
                }

                else {
                    it++;
                }
            }
            for (auto it:t){
                action(it.file, it.filePath, it.fs, it.ft);
            }
            t.clear();
            std::vector<struct tmp> d;
            // Check if a file was created or modified
            for(auto &file : std::filesystem::recursive_directory_iterator(path_to_watch)) {
                auto current_file_last_write_time = std::filesystem::last_write_time(file);
                // File creation
                // First send all created directories
                if(!contains(file.path().string())) {
                    paths[file.path().string()].last_mod = current_file_last_write_time;
                    paths[file.path().string()].type = (file.is_directory()) ? FileType::directory : FileType::file;
                    if (paths[file.path().string()].type == FileType::directory){
                        /*struct tmp x;
                        x.file = file.path().filename().string();
                        x.filePath = file.path().string();
                        x.fs = FileStatus::created;
                        x.ft = FileType::directory;
                        d.push_back(x);*/
                        action(file.path().filename().string(), file.path().string(), FileStatus::created, FileType::directory);
                    }else{
                        struct tmp x;
                        x.file = file.path().filename().string();
                        x.filePath = file.path().string();
                        x.fs = FileStatus::created;
                        x.ft = FileType::file;
                        t.push_back(x);
                    }

                } else {
                    // File modification
                    if(paths[file.path().string()].last_mod != current_file_last_write_time) {
                        paths[file.path().string()].last_mod = current_file_last_write_time;
                        action(file.path().filename().string(), file.path().string(), FileStatus::modified, (file.is_directory()) ? FileType::directory : FileType::file);
                    }
                }
            }

            // send directory backwords
            /*
            for(auto it = d.end(); it != d.begin(); it--){
                action(it->file, it->filePath, it->fs, it->ft);
            }*/

            // At the end send all created files
            for (auto it:t){
                action(it.file, it.filePath, it.fs, it.ft);
            }
            t.clear();
        }
    }

    void stop(){
        running = false;
    }

};


#endif //PDS_PROJECT_CLIENT_FILEWATCHER_H
