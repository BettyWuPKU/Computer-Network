# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.19

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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.19.4/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.19.4/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/bettywu/Desktop/assignment2-rtp

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/bettywu/Desktop/assignment2-rtp/build

# Include any dependencies generated for this target.
include CMakeFiles/test_sender.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/test_sender.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_sender.dir/flags.make

CMakeFiles/test_sender.dir/src/rtp.c.o: CMakeFiles/test_sender.dir/flags.make
CMakeFiles/test_sender.dir/src/rtp.c.o: ../src/rtp.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/bettywu/Desktop/assignment2-rtp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/test_sender.dir/src/rtp.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/test_sender.dir/src/rtp.c.o -c /Users/bettywu/Desktop/assignment2-rtp/src/rtp.c

CMakeFiles/test_sender.dir/src/rtp.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/test_sender.dir/src/rtp.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/bettywu/Desktop/assignment2-rtp/src/rtp.c > CMakeFiles/test_sender.dir/src/rtp.c.i

CMakeFiles/test_sender.dir/src/rtp.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/test_sender.dir/src/rtp.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/bettywu/Desktop/assignment2-rtp/src/rtp.c -o CMakeFiles/test_sender.dir/src/rtp.c.s

CMakeFiles/test_sender.dir/src/util.c.o: CMakeFiles/test_sender.dir/flags.make
CMakeFiles/test_sender.dir/src/util.c.o: ../src/util.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/bettywu/Desktop/assignment2-rtp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/test_sender.dir/src/util.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/test_sender.dir/src/util.c.o -c /Users/bettywu/Desktop/assignment2-rtp/src/util.c

CMakeFiles/test_sender.dir/src/util.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/test_sender.dir/src/util.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/bettywu/Desktop/assignment2-rtp/src/util.c > CMakeFiles/test_sender.dir/src/util.c.i

CMakeFiles/test_sender.dir/src/util.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/test_sender.dir/src/util.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/bettywu/Desktop/assignment2-rtp/src/util.c -o CMakeFiles/test_sender.dir/src/util.c.s

CMakeFiles/test_sender.dir/src/test_sender.c.o: CMakeFiles/test_sender.dir/flags.make
CMakeFiles/test_sender.dir/src/test_sender.c.o: ../src/test_sender.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/bettywu/Desktop/assignment2-rtp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/test_sender.dir/src/test_sender.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/test_sender.dir/src/test_sender.c.o -c /Users/bettywu/Desktop/assignment2-rtp/src/test_sender.c

CMakeFiles/test_sender.dir/src/test_sender.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/test_sender.dir/src/test_sender.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/bettywu/Desktop/assignment2-rtp/src/test_sender.c > CMakeFiles/test_sender.dir/src/test_sender.c.i

CMakeFiles/test_sender.dir/src/test_sender.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/test_sender.dir/src/test_sender.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/bettywu/Desktop/assignment2-rtp/src/test_sender.c -o CMakeFiles/test_sender.dir/src/test_sender.c.s

# Object files for target test_sender
test_sender_OBJECTS = \
"CMakeFiles/test_sender.dir/src/rtp.c.o" \
"CMakeFiles/test_sender.dir/src/util.c.o" \
"CMakeFiles/test_sender.dir/src/test_sender.c.o"

# External object files for target test_sender
test_sender_EXTERNAL_OBJECTS =

test_sender: CMakeFiles/test_sender.dir/src/rtp.c.o
test_sender: CMakeFiles/test_sender.dir/src/util.c.o
test_sender: CMakeFiles/test_sender.dir/src/test_sender.c.o
test_sender: CMakeFiles/test_sender.dir/build.make
test_sender: CMakeFiles/test_sender.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/bettywu/Desktop/assignment2-rtp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C executable test_sender"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_sender.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_sender.dir/build: test_sender

.PHONY : CMakeFiles/test_sender.dir/build

CMakeFiles/test_sender.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_sender.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_sender.dir/clean

CMakeFiles/test_sender.dir/depend:
	cd /Users/bettywu/Desktop/assignment2-rtp/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/bettywu/Desktop/assignment2-rtp /Users/bettywu/Desktop/assignment2-rtp /Users/bettywu/Desktop/assignment2-rtp/build /Users/bettywu/Desktop/assignment2-rtp/build /Users/bettywu/Desktop/assignment2-rtp/build/CMakeFiles/test_sender.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test_sender.dir/depend

