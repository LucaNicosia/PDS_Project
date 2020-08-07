cmake_minimum_required(VERSION 3.16)
project(PDS_Project_Client)

set(CMAKE_CXX_STANDARD 17)

set(GCC_COVERAGE_COMPILE_FLAGS "-pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

add_executable(PDS_Project_Client main.cpp ./TCP_Socket/Socket.cpp ./TCP_Socket/Socket.h DB/Database.cpp DB/Database.h)
target_link_libraries(PDS_Project_Client LINK_PUBLIC sqlite3)