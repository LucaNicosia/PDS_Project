# PDS_Project (m1)
###### Students:  Giuseppe Toscano (280086) - Luca Nicosia (269565)
###### GitHub link: https://github.com/LucaNicosia/PDS_Project  

#### 1. Needed packages
- [SQLITE3](https://www.sqlite.org/cintro.html): `sudo apt-get install libsqlite3-dev`
- [CRYPTOPP](https://www.cryptopp.com/wiki/SHA): `sudo apt-get install libcrypto++-dev libcrypto++-doc libcrypto++-utils`

#### 2. Needed initial folders and files
      /client_folder
        /DB
        /executable_dir
          /client_directory
          PDS_Project_Client (executable)
        /Log
          log.txt

      /server_folder
        /DB
          /users
        /executable_dir
          /server_directory
          PDS_Project_Server (executable)
        /Log
          log.txt

###### 2.1 Description of initial folders
- **DB**: contains all db files (e.g. user1.db, user2.db, ...)
- **executable_dir**: contains the executable file and **client_directory/server_directory** that contains users data
- **Log**: contains the log file

##### 3. Parameters of the executable files
- **PDS_Project_Server**: `PORT` (e.g. 5111)
- **PDS_Project_Client**: `PORT username password FETCH/RESTORE`(e.g. 5111 user1 pass FETCH)
  >You can execute our program in two different modes:
    1. FETCH: *replicates all user actions on client directory in server directory*
    2. RESTORE: *delete all local files and restore  all data contained in the server directory*


##### 4. Internal structures
###### 4.1 Database
Two db types:
- **_username_.db**
  - **directory** (path, name)
  - **file** (path, name, hash)
- **users.db**
  - **users** (username, password, salt)

> The field *password* of users is not the real password but the salted digest computed with the library *cryptopp*

###### 4.2 Communication protocol

- **FETCH MODE**:
        CLIENT                                                SERVER                                                                                       
        CONNECT username password mode ->
                                                              <- CONNECT-OK
        CONNECT-OK ->
                                                              if (!correct password)
                                                              <- wrong username or password
        return
                                                              else
                                                              <- DIGEST server_db_digest
                                                              endif
        if (client_db_digest == server_db_digest)
        Database up to date ->                                 
                                                              <- server_db_ok
        else
        GET-DB ->   
                                                              <- sendFile(server_db)
        ... compare the two databases ...
        for each different directory:
        DIR path action ->              
                                                              <- DONE

        for each different file:
        FILE path action ->
                                                              if (action == 'erased')
                                                              <- DONE
                                                              else  
                                                              <- READY
                                                              endif
        if (action != 'erased')
        FILE path length_file ->                    
                                                              <- OK
        sendFile(path) ->             
                                                              <- DONE
        endif
        endif

- **RESTORE MODE**: like FETCH MODE but the file/directory transfering is in the opposite direction.                                         

###### 4.3  Communication.h
This library contains primitives to send and receive messages and files between two hosts.

###### 4.4  File.h, File.cpp, Directory.h and Directory.cpp
These two classes are used to represent a simplified view of the filesystem with some more addons (e.g. file digest).

###### 4.5  Socket.h, Socket.cpp, SocketServer.h and SocketServer.cpp
These two classes are used to perform a TCP connection between two hosts. There is a customizable timeout for each message.

###### 4.5  MyCryptoLibrary.h
This library is used to compute the digest of a file or a string.

###### 4.6  Utilities.h
This library contains the Logger class that is used to write on the log file.

###### 4.7  FileWatcher.h
This library is used to monitor a specific directory in the client space and react to each modification.

###### 4.8  Database.h
This library is used to perform queries on databases using *SQLITE3*.

###### 4.9  Exceptions.h
This library contains definitions of different types of exception:
  - *general_exception*
  - *socket_exception*
  - *filesystem_exception* (derived from *general_exception*)
  - *database_exception* (derived from *general_exception*)

##### 5. Processes and Threads
###### 5.1 Processes

###### 5.1.1 *Client*
At the beginning the client process performs a fork():
  - *father*: display on console the child status received from the pipe (e.g. 'connection succeed', e.g. 'wrong username or password'). It is used to make the child a zombie process
  - *child*: executes the main program and sends its status via pipe

###### 5.1.2 *Server*
At the beginning the server process performs a fork():
  - *father*: dies (it is used to make the child a zombie process)
  - *child*: executes the main program

###### 5.2 Threads

###### 5.2.1 *Client*
  - *main thread*: executes the main program and creates the below threads
  - *FileWatcher thread*: setup and starts the FileWatcher
  - *monitor thread*: monitors the status of the FileWatcher and opens or closes server connection  

###### 5.2.2 *Server*
  - *main thread*: listens on socket port and creates the below threads
  - *user thread*: a thread is instantiated for each user that connects to the server (until the number of connected users reach a threshold). This thread executes the main program.
