# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /snap/clion/121/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/121/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/giuseppetoscano/Desktop/PDS_Project/Client

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/giuseppetoscano/Desktop/PDS_Project/Client/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/PDS_Progetto.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/PDS_Progetto.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/PDS_Progetto.dir/flags.make

CMakeFiles/PDS_Progetto.dir/main.cpp.o: CMakeFiles/PDS_Progetto.dir/flags.make
CMakeFiles/PDS_Progetto.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/giuseppetoscano/Desktop/PDS_Project/Client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/PDS_Progetto.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/PDS_Progetto.dir/main.cpp.o -c /home/giuseppetoscano/Desktop/PDS_Project/Client/main.cpp

CMakeFiles/PDS_Progetto.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/PDS_Progetto.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/giuseppetoscano/Desktop/PDS_Project/Client/main.cpp > CMakeFiles/PDS_Progetto.dir/main.cpp.i

CMakeFiles/PDS_Progetto.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/PDS_Progetto.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/giuseppetoscano/Desktop/PDS_Project/Client/main.cpp -o CMakeFiles/PDS_Progetto.dir/main.cpp.s

CMakeFiles/PDS_Progetto.dir/TCP_Socket/Socket.cpp.o: CMakeFiles/PDS_Progetto.dir/flags.make
CMakeFiles/PDS_Progetto.dir/TCP_Socket/Socket.cpp.o: ../TCP_Socket/Socket.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/giuseppetoscano/Desktop/PDS_Project/Client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/PDS_Progetto.dir/TCP_Socket/Socket.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/PDS_Progetto.dir/TCP_Socket/Socket.cpp.o -c /home/giuseppetoscano/Desktop/PDS_Project/Client/TCP_Socket/Socket.cpp

CMakeFiles/PDS_Progetto.dir/TCP_Socket/Socket.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/PDS_Progetto.dir/TCP_Socket/Socket.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/giuseppetoscano/Desktop/PDS_Project/Client/TCP_Socket/Socket.cpp > CMakeFiles/PDS_Progetto.dir/TCP_Socket/Socket.cpp.i

CMakeFiles/PDS_Progetto.dir/TCP_Socket/Socket.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/PDS_Progetto.dir/TCP_Socket/Socket.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/giuseppetoscano/Desktop/PDS_Project/Client/TCP_Socket/Socket.cpp -o CMakeFiles/PDS_Progetto.dir/TCP_Socket/Socket.cpp.s

CMakeFiles/PDS_Progetto.dir/DB/Database.cpp.o: CMakeFiles/PDS_Progetto.dir/flags.make
CMakeFiles/PDS_Progetto.dir/DB/Database.cpp.o: ../DB/Database.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/giuseppetoscano/Desktop/PDS_Project/Client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/PDS_Progetto.dir/DB/Database.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/PDS_Progetto.dir/DB/Database.cpp.o -c /home/giuseppetoscano/Desktop/PDS_Project/Client/DB/Database.cpp

CMakeFiles/PDS_Progetto.dir/DB/Database.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/PDS_Progetto.dir/DB/Database.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/giuseppetoscano/Desktop/PDS_Project/Client/DB/Database.cpp > CMakeFiles/PDS_Progetto.dir/DB/Database.cpp.i

CMakeFiles/PDS_Progetto.dir/DB/Database.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/PDS_Progetto.dir/DB/Database.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/giuseppetoscano/Desktop/PDS_Project/Client/DB/Database.cpp -o CMakeFiles/PDS_Progetto.dir/DB/Database.cpp.s

# Object files for target PDS_Progetto
PDS_Progetto_OBJECTS = \
"CMakeFiles/PDS_Progetto.dir/main.cpp.o" \
"CMakeFiles/PDS_Progetto.dir/TCP_Socket/Socket.cpp.o" \
"CMakeFiles/PDS_Progetto.dir/DB/Database.cpp.o"

# External object files for target PDS_Progetto
PDS_Progetto_EXTERNAL_OBJECTS =

PDS_Progetto: CMakeFiles/PDS_Progetto.dir/main.cpp.o
PDS_Progetto: CMakeFiles/PDS_Progetto.dir/TCP_Socket/Socket.cpp.o
PDS_Progetto: CMakeFiles/PDS_Progetto.dir/DB/Database.cpp.o
PDS_Progetto: CMakeFiles/PDS_Progetto.dir/build.make
PDS_Progetto: CMakeFiles/PDS_Progetto.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/giuseppetoscano/Desktop/PDS_Project/Client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable PDS_Progetto"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/PDS_Progetto.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/PDS_Progetto.dir/build: PDS_Progetto

.PHONY : CMakeFiles/PDS_Progetto.dir/build

CMakeFiles/PDS_Progetto.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/PDS_Progetto.dir/cmake_clean.cmake
.PHONY : CMakeFiles/PDS_Progetto.dir/clean

CMakeFiles/PDS_Progetto.dir/depend:
	cd /home/giuseppetoscano/Desktop/PDS_Project/Client/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/giuseppetoscano/Desktop/PDS_Project/Client /home/giuseppetoscano/Desktop/PDS_Project/Client /home/giuseppetoscano/Desktop/PDS_Project/Client/cmake-build-debug /home/giuseppetoscano/Desktop/PDS_Project/Client/cmake-build-debug /home/giuseppetoscano/Desktop/PDS_Project/Client/cmake-build-debug/CMakeFiles/PDS_Progetto.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/PDS_Progetto.dir/depend

