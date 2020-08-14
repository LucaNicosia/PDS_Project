cmake_minimum_required(VERSION 3.16)
project(PDS_Project_Server)

set(CMAKE_CXX_STANDARD 17)

set(GCC_COVERAGE_COMPILE_FLAGS "-pthread -lcryptopp" )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

find_library(CryptoPP libcryptopp.so REQUIRED)

add_executable(PDS_Project_Server main.cpp ./TCP_Socket/Socket.cpp ./TCP_Socket/Socket.h ./TCP_Socket/SocketServer.cpp ./TCP_Socket/SocketServer.h ./FileManager/File.cpp ./FileManager/File.h ./FileManager/Directory.cpp ./FileManager/Directory.h DB/Database.h ./Communication/Communication.h)
target_link_libraries(PDS_Project_Server LINK_PUBLIC sqlite3 ${CryptoPP})



