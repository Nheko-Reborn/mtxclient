# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/nicolas/Dokumente/devel/open-source/mtxclient

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/nicolas/Dokumente/devel/open-source/mtxclient

# Include any dependencies generated for this target.
include examples/CMakeFiles/simple_bot.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/simple_bot.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/simple_bot.dir/flags.make

examples/CMakeFiles/simple_bot.dir/simple_bot.cpp.o: examples/CMakeFiles/simple_bot.dir/flags.make
examples/CMakeFiles/simple_bot.dir/simple_bot.cpp.o: examples/simple_bot.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nicolas/Dokumente/devel/open-source/mtxclient/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object examples/CMakeFiles/simple_bot.dir/simple_bot.cpp.o"
	cd /home/nicolas/Dokumente/devel/open-source/mtxclient/examples && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/simple_bot.dir/simple_bot.cpp.o -c /home/nicolas/Dokumente/devel/open-source/mtxclient/examples/simple_bot.cpp

examples/CMakeFiles/simple_bot.dir/simple_bot.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/simple_bot.dir/simple_bot.cpp.i"
	cd /home/nicolas/Dokumente/devel/open-source/mtxclient/examples && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/nicolas/Dokumente/devel/open-source/mtxclient/examples/simple_bot.cpp > CMakeFiles/simple_bot.dir/simple_bot.cpp.i

examples/CMakeFiles/simple_bot.dir/simple_bot.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/simple_bot.dir/simple_bot.cpp.s"
	cd /home/nicolas/Dokumente/devel/open-source/mtxclient/examples && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/nicolas/Dokumente/devel/open-source/mtxclient/examples/simple_bot.cpp -o CMakeFiles/simple_bot.dir/simple_bot.cpp.s

# Object files for target simple_bot
simple_bot_OBJECTS = \
"CMakeFiles/simple_bot.dir/simple_bot.cpp.o"

# External object files for target simple_bot
simple_bot_EXTERNAL_OBJECTS =

examples/simple_bot: examples/CMakeFiles/simple_bot.dir/simple_bot.cpp.o
examples/simple_bot: examples/CMakeFiles/simple_bot.dir/build.make
examples/simple_bot: libmatrix_client.so.0.5.1
examples/simple_bot: /usr/lib64/libcoeurl.so
examples/simple_bot: /usr/lib64/libssl.so
examples/simple_bot: /usr/lib64/libcrypto.so
examples/simple_bot: /usr/lib64/libolm.so.3.1.5
examples/simple_bot: examples/CMakeFiles/simple_bot.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/nicolas/Dokumente/devel/open-source/mtxclient/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable simple_bot"
	cd /home/nicolas/Dokumente/devel/open-source/mtxclient/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/simple_bot.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/simple_bot.dir/build: examples/simple_bot

.PHONY : examples/CMakeFiles/simple_bot.dir/build

examples/CMakeFiles/simple_bot.dir/clean:
	cd /home/nicolas/Dokumente/devel/open-source/mtxclient/examples && $(CMAKE_COMMAND) -P CMakeFiles/simple_bot.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/simple_bot.dir/clean

examples/CMakeFiles/simple_bot.dir/depend:
	cd /home/nicolas/Dokumente/devel/open-source/mtxclient && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/nicolas/Dokumente/devel/open-source/mtxclient /home/nicolas/Dokumente/devel/open-source/mtxclient/examples /home/nicolas/Dokumente/devel/open-source/mtxclient /home/nicolas/Dokumente/devel/open-source/mtxclient/examples /home/nicolas/Dokumente/devel/open-source/mtxclient/examples/CMakeFiles/simple_bot.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/CMakeFiles/simple_bot.dir/depend

