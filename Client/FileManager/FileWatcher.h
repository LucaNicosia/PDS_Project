//
// Created by root on 09/08/20.
//

#ifndef PDS_PROJECT_CLIENT_FILEWATCHER_H
#define PDS_PROJECT_CLIENT_FILEWATCHER_H

#include <filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
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
    std::unordered_map<std::string, FileStruct> paths;
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

    void start(const std::function<void (std::string, FileStatus, FileType)> &action) {
        while(running) {
            // Wait for "delay" milliseconds
            std::this_thread::sleep_for(delay);
            if(!running) return; // if when i wake up 'running' is 'false', don't do the 'action' function
            auto it = paths.begin();
            // check if a file was deleted
            while (it != paths.end()) {
                if (!std::filesystem::exists(it->first)) {
                    action(it->first, FileStatus::erased,it->second.type); // mando se è directory o file
                    it = paths.erase(it);
                    }
                else {
                    it++;
                }
            }
            // Check if a file was created or modified
            for(auto &file : std::filesystem::recursive_directory_iterator(path_to_watch)) {
                auto current_file_last_write_time = std::filesystem::last_write_time(file);
                // File creation

                if(!contains(file.path().string())) {
                    paths[file.path().string()].last_mod = current_file_last_write_time;
                    paths[file.path().string()].type = (file.is_directory()) ? FileType::directory : FileType::file;
                    action(file.path().string(), FileStatus::created, (file.is_directory()) ? FileType::directory : FileType::file);
                } else {
                    // File modification
                    if(paths[file.path().string()].last_mod != current_file_last_write_time) {
                        paths[file.path().string()].last_mod = current_file_last_write_time;
                        action(file.path().string(), FileStatus::modified, (file.is_directory()) ? FileType::directory : FileType::file);
                    }
                }
            }
        }
    }

    void stop(){
        running = false;
    }

};


#endif //PDS_PROJECT_CLIENT_FILEWATCHER_H
