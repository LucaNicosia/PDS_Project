
#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <filesystem>
#include <chrono>
#include <thread>
#include <map>
#include <string>
#include <functional>
#include <mutex>

#define DEBUG 0

// Define available file changes
enum class FileStatus {created, modified, erased, none};
enum class FileType {file, directory};
enum class FileWatcher_state {ready, mod_found, ended};

struct action_data {
    std::string file;
    std::string filePath;
    FileStatus fs;
    FileType ft;
};

class FileWatcher {
    class FileStruct{
    public:
        std::filesystem::file_time_type last_mod;
        FileType type;
        FileStruct(std::filesystem::file_time_type last_mod, FileType type): last_mod(last_mod), type(type) {}
        FileStruct(){};
    };
    std::mutex m;
    FileWatcher_state cur_state;
    FileWatcher_state last_state;
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

    FileWatcher(){}

    FileWatcher(std::string path_to_watch, std::chrono::duration<int, std::milli> delay) {
        this->set(path_to_watch,delay);
    }

    ~FileWatcher(){
        if (DEBUG)
            std::cout<<"Destroying FileWatcher"<<std::endl;
        this->stop();
    }

    void set(std::string path_to_watch,std::chrono::duration<int, std::milli> delay){
        this->running = true;
        this->path_to_watch= path_to_watch;
        this->delay = delay;
        this->cur_state = FileWatcher_state::ready;
        this->last_state = FileWatcher_state::ready;
        for(auto &file : std::filesystem::recursive_directory_iterator(path_to_watch)) {
            paths[file.path().string()] = FileStruct(std::filesystem::last_write_time(file),(file.is_directory())?FileType::directory:FileType::file);
        }
    }

    void start(const std::function<void (std::string, std::string, FileStatus, FileType)> &action) {
        std::vector<struct action_data> t;
        while(isRunning()) {
            setCurState(FileWatcher_state::ready);
            // Wait for "delay" milliseconds
            std::this_thread::sleep_for(delay);
            if(!isRunning()) return; // If when i wake up 'running' is 'false', don't do the 'action' function
            auto it = paths.begin();
            // Check if a file was deleted
            while (it != paths.end() ) {
                // Send files
                if (!std::filesystem::exists(std::filesystem::status(it->first))){
                    if (paths[it->first].type == FileType::file) {
                        setCurState(FileWatcher_state::mod_found);
                        action(it->first, it->first, FileStatus::erased, it->second.type); // mando se è file
                        it = paths.erase(it);
                    }
                    else {
                        // Preparing directories to send at the end
                        struct action_data x;
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
            for (int i = t.size()-1; i >= 0; i--){
                setCurState(FileWatcher_state::mod_found);
                action(t[i].file, t[i].filePath, t[i].fs, t[i].ft);
            }
            t.clear();
            std::map<std::string, struct action_data> d;
            // Check if a file was created or modified
            for(auto &file : std::filesystem::recursive_directory_iterator(path_to_watch)) {
                auto current_file_last_write_time = std::filesystem::last_write_time(file);
                // File creation
                // First send all created directories
                if(!contains(file.path().string())) {
                    paths[file.path().string()].last_mod = current_file_last_write_time;
                    paths[file.path().string()].type = (file.is_directory()) ? FileType::directory : FileType::file;
                    if (paths[file.path().string()].type == FileType::directory){
                        setCurState(FileWatcher_state::mod_found);
                        action(file.path().filename().string(), file.path().string(), FileStatus::created, FileType::directory);
                    }else{
                        struct action_data x;
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
                        setCurState(FileWatcher_state::mod_found);
                        action(file.path().filename().string(), file.path().string(), FileStatus::modified, (file.is_directory()) ? FileType::directory : FileType::file);
                    }
                }
            }

            // At the end send all created files
            for (int i=0;i<t.size();i++){
                setCurState(FileWatcher_state::mod_found);
                action(t[i].file, t[i].filePath, t[i].fs, t[i].ft);
            }
            t.clear();
            setCurState(FileWatcher_state::ended);
        }
    }

    void stop(){
        setRunning(false);
    }

    FileWatcher_state getCurState() {
        std::lock_guard<std::mutex>lg(m);
        return cur_state;
    }

    FileWatcher_state getLastState() {
        std::lock_guard<std::mutex>lg(m);
        return last_state;
    }

    void getAllState(FileWatcher_state& last, FileWatcher_state& cur){
        std::lock_guard<std::mutex>lg(m);
        last = last_state;
        cur = cur_state;
    }

    void setCurState(FileWatcher_state curState) {
        std::lock_guard<std::mutex>lg(m);
        last_state = cur_state;
        cur_state = curState;
    }

    bool isRunning() {
        std::lock_guard<std::mutex>lg(m);
        return running;
    }

    void setRunning(bool running) {
        std::lock_guard<std::mutex>lg(m);
        FileWatcher::running = running;
    }

};


#endif //FILEWATCHER_H
