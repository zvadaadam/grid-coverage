# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

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
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/adam.zvada/Documents/School/coverage

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/adam.zvada/Documents/School/coverage/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/coverage.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/coverage.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/coverage.dir/flags.make

CMakeFiles/coverage.dir/src/main.cpp.o: CMakeFiles/coverage.dir/flags.make
CMakeFiles/coverage.dir/src/main.cpp.o: ../src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/adam.zvada/Documents/School/coverage/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/coverage.dir/src/main.cpp.o"
	/usr/local/bin/g++-8  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/coverage.dir/src/main.cpp.o -c /Users/adam.zvada/Documents/School/coverage/src/main.cpp

CMakeFiles/coverage.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/coverage.dir/src/main.cpp.i"
	/usr/local/bin/g++-8 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/adam.zvada/Documents/School/coverage/src/main.cpp > CMakeFiles/coverage.dir/src/main.cpp.i

CMakeFiles/coverage.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/coverage.dir/src/main.cpp.s"
	/usr/local/bin/g++-8 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/adam.zvada/Documents/School/coverage/src/main.cpp -o CMakeFiles/coverage.dir/src/main.cpp.s

# Object files for target coverage
coverage_OBJECTS = \
"CMakeFiles/coverage.dir/src/main.cpp.o"

# External object files for target coverage
coverage_EXTERNAL_OBJECTS =

coverage: CMakeFiles/coverage.dir/src/main.cpp.o
coverage: CMakeFiles/coverage.dir/build.make
coverage: CMakeFiles/coverage.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/adam.zvada/Documents/School/coverage/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable coverage"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/coverage.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/coverage.dir/build: coverage

.PHONY : CMakeFiles/coverage.dir/build

CMakeFiles/coverage.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/coverage.dir/cmake_clean.cmake
.PHONY : CMakeFiles/coverage.dir/clean

CMakeFiles/coverage.dir/depend:
	cd /Users/adam.zvada/Documents/School/coverage/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/adam.zvada/Documents/School/coverage /Users/adam.zvada/Documents/School/coverage /Users/adam.zvada/Documents/School/coverage/cmake-build-debug /Users/adam.zvada/Documents/School/coverage/cmake-build-debug /Users/adam.zvada/Documents/School/coverage/cmake-build-debug/CMakeFiles/coverage.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/coverage.dir/depend
