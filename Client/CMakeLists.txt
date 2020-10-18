cmake_minimum_required(VERSION 3.16)
project(PDS_Project_Client)

set(CMAKE_CXX_STANDARD 17)

set(GCC_COVERAGE_COMPILE_FLAGS "-pthread -lcryptopp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

find_library(CryptoPP libcryptopp.so REQUIRED)

add_executable(PDS_Project_Client main.cpp Entities/Socket/Socket.cpp Entities/Socket/Socket.h Entities/File/File.cpp Entities/File/File.h Entities/Directory/Directory.cpp Entities/Directory/Directory.h "Usefull functions/Communication/Communication.h" "Usefull functions/main_functions.h" "Usefull functions/constants.h" Entities/Exceptions/MyExceptions.h )
target_link_libraries(PDS_Project_Client LINK_PUBLIC sqlite3 ${CryptoPP})
