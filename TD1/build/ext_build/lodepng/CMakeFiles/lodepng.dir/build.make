# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /net/cremi/jimgouraud/SIA/TD1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /net/cremi/jimgouraud/SIA/TD1/build

# Include any dependencies generated for this target.
include ext_build/lodepng/CMakeFiles/lodepng.dir/depend.make

# Include the progress variables for this target.
include ext_build/lodepng/CMakeFiles/lodepng.dir/progress.make

# Include the compile flags for this target's objects.
include ext_build/lodepng/CMakeFiles/lodepng.dir/flags.make

ext_build/lodepng/CMakeFiles/lodepng.dir/lodepng.cpp.o: ext_build/lodepng/CMakeFiles/lodepng.dir/flags.make
ext_build/lodepng/CMakeFiles/lodepng.dir/lodepng.cpp.o: ../ext/lodepng/lodepng.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/net/cremi/jimgouraud/SIA/TD1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object ext_build/lodepng/CMakeFiles/lodepng.dir/lodepng.cpp.o"
	cd /net/cremi/jimgouraud/SIA/TD1/build/ext_build/lodepng && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lodepng.dir/lodepng.cpp.o -c /net/cremi/jimgouraud/SIA/TD1/ext/lodepng/lodepng.cpp

ext_build/lodepng/CMakeFiles/lodepng.dir/lodepng.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lodepng.dir/lodepng.cpp.i"
	cd /net/cremi/jimgouraud/SIA/TD1/build/ext_build/lodepng && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /net/cremi/jimgouraud/SIA/TD1/ext/lodepng/lodepng.cpp > CMakeFiles/lodepng.dir/lodepng.cpp.i

ext_build/lodepng/CMakeFiles/lodepng.dir/lodepng.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lodepng.dir/lodepng.cpp.s"
	cd /net/cremi/jimgouraud/SIA/TD1/build/ext_build/lodepng && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /net/cremi/jimgouraud/SIA/TD1/ext/lodepng/lodepng.cpp -o CMakeFiles/lodepng.dir/lodepng.cpp.s

ext_build/lodepng/CMakeFiles/lodepng.dir/lodepng.cpp.o.requires:

.PHONY : ext_build/lodepng/CMakeFiles/lodepng.dir/lodepng.cpp.o.requires

ext_build/lodepng/CMakeFiles/lodepng.dir/lodepng.cpp.o.provides: ext_build/lodepng/CMakeFiles/lodepng.dir/lodepng.cpp.o.requires
	$(MAKE) -f ext_build/lodepng/CMakeFiles/lodepng.dir/build.make ext_build/lodepng/CMakeFiles/lodepng.dir/lodepng.cpp.o.provides.build
.PHONY : ext_build/lodepng/CMakeFiles/lodepng.dir/lodepng.cpp.o.provides

ext_build/lodepng/CMakeFiles/lodepng.dir/lodepng.cpp.o.provides.build: ext_build/lodepng/CMakeFiles/lodepng.dir/lodepng.cpp.o


# Object files for target lodepng
lodepng_OBJECTS = \
"CMakeFiles/lodepng.dir/lodepng.cpp.o"

# External object files for target lodepng
lodepng_EXTERNAL_OBJECTS =

ext_build/lodepng/liblodepng.a: ext_build/lodepng/CMakeFiles/lodepng.dir/lodepng.cpp.o
ext_build/lodepng/liblodepng.a: ext_build/lodepng/CMakeFiles/lodepng.dir/build.make
ext_build/lodepng/liblodepng.a: ext_build/lodepng/CMakeFiles/lodepng.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/net/cremi/jimgouraud/SIA/TD1/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library liblodepng.a"
	cd /net/cremi/jimgouraud/SIA/TD1/build/ext_build/lodepng && $(CMAKE_COMMAND) -P CMakeFiles/lodepng.dir/cmake_clean_target.cmake
	cd /net/cremi/jimgouraud/SIA/TD1/build/ext_build/lodepng && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lodepng.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
ext_build/lodepng/CMakeFiles/lodepng.dir/build: ext_build/lodepng/liblodepng.a

.PHONY : ext_build/lodepng/CMakeFiles/lodepng.dir/build

ext_build/lodepng/CMakeFiles/lodepng.dir/requires: ext_build/lodepng/CMakeFiles/lodepng.dir/lodepng.cpp.o.requires

.PHONY : ext_build/lodepng/CMakeFiles/lodepng.dir/requires

ext_build/lodepng/CMakeFiles/lodepng.dir/clean:
	cd /net/cremi/jimgouraud/SIA/TD1/build/ext_build/lodepng && $(CMAKE_COMMAND) -P CMakeFiles/lodepng.dir/cmake_clean.cmake
.PHONY : ext_build/lodepng/CMakeFiles/lodepng.dir/clean

ext_build/lodepng/CMakeFiles/lodepng.dir/depend:
	cd /net/cremi/jimgouraud/SIA/TD1/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /net/cremi/jimgouraud/SIA/TD1 /net/cremi/jimgouraud/SIA/TD1/ext/lodepng /net/cremi/jimgouraud/SIA/TD1/build /net/cremi/jimgouraud/SIA/TD1/build/ext_build/lodepng /net/cremi/jimgouraud/SIA/TD1/build/ext_build/lodepng/CMakeFiles/lodepng.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : ext_build/lodepng/CMakeFiles/lodepng.dir/depend

