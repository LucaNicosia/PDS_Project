cmake_minimum_required(VERSION 3.16)
project(PDS_Progetto)

set(CMAKE_CXX_STANDARD 17)

set(GCC_COVERAGE_COMPILE_FLAGS "-pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

add_executable(PDS_Progetto main.cpp ./TCP_Socket/Socket.cpp ./TCP_Socket/Socket.h)
target_link_libraries(PDS_Progetto LINK_PUBLIC sqlite3)