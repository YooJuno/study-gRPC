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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/juno/grpc/examples/cpp/jpeg

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/juno/grpc/examples/cpp/jpeg/cmake/build

# Include any dependencies generated for this target.
include CMakeFiles/jp_grpc_proto.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/jp_grpc_proto.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/jp_grpc_proto.dir/flags.make

jpeg.pb.cc: ../../protos/jpeg.proto
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/juno/grpc/examples/cpp/jpeg/cmake/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating jpeg.pb.cc, jpeg.pb.h, jpeg.grpc.pb.cc, jpeg.grpc.pb.h"
	/bin/protoc-26.1.0 --grpc_out /home/juno/grpc/examples/cpp/jpeg/cmake/build --cpp_out /home/juno/grpc/examples/cpp/jpeg/cmake/build -I /home/juno/grpc/examples/cpp/jpeg/protos --plugin=protoc-gen-grpc="/bin/grpc_cpp_plugin" /home/juno/grpc/examples/cpp/jpeg/protos/jpeg.proto

jpeg.pb.h: jpeg.pb.cc
	@$(CMAKE_COMMAND) -E touch_nocreate jpeg.pb.h

jpeg.grpc.pb.cc: jpeg.pb.cc
	@$(CMAKE_COMMAND) -E touch_nocreate jpeg.grpc.pb.cc

jpeg.grpc.pb.h: jpeg.pb.cc
	@$(CMAKE_COMMAND) -E touch_nocreate jpeg.grpc.pb.h

CMakeFiles/jp_grpc_proto.dir/jpeg.grpc.pb.cc.o: CMakeFiles/jp_grpc_proto.dir/flags.make
CMakeFiles/jp_grpc_proto.dir/jpeg.grpc.pb.cc.o: jpeg.grpc.pb.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/juno/grpc/examples/cpp/jpeg/cmake/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/jp_grpc_proto.dir/jpeg.grpc.pb.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/jp_grpc_proto.dir/jpeg.grpc.pb.cc.o -c /home/juno/grpc/examples/cpp/jpeg/cmake/build/jpeg.grpc.pb.cc

CMakeFiles/jp_grpc_proto.dir/jpeg.grpc.pb.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/jp_grpc_proto.dir/jpeg.grpc.pb.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/juno/grpc/examples/cpp/jpeg/cmake/build/jpeg.grpc.pb.cc > CMakeFiles/jp_grpc_proto.dir/jpeg.grpc.pb.cc.i

CMakeFiles/jp_grpc_proto.dir/jpeg.grpc.pb.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/jp_grpc_proto.dir/jpeg.grpc.pb.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/juno/grpc/examples/cpp/jpeg/cmake/build/jpeg.grpc.pb.cc -o CMakeFiles/jp_grpc_proto.dir/jpeg.grpc.pb.cc.s

CMakeFiles/jp_grpc_proto.dir/jpeg.pb.cc.o: CMakeFiles/jp_grpc_proto.dir/flags.make
CMakeFiles/jp_grpc_proto.dir/jpeg.pb.cc.o: jpeg.pb.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/juno/grpc/examples/cpp/jpeg/cmake/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/jp_grpc_proto.dir/jpeg.pb.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/jp_grpc_proto.dir/jpeg.pb.cc.o -c /home/juno/grpc/examples/cpp/jpeg/cmake/build/jpeg.pb.cc

CMakeFiles/jp_grpc_proto.dir/jpeg.pb.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/jp_grpc_proto.dir/jpeg.pb.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/juno/grpc/examples/cpp/jpeg/cmake/build/jpeg.pb.cc > CMakeFiles/jp_grpc_proto.dir/jpeg.pb.cc.i

CMakeFiles/jp_grpc_proto.dir/jpeg.pb.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/jp_grpc_proto.dir/jpeg.pb.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/juno/grpc/examples/cpp/jpeg/cmake/build/jpeg.pb.cc -o CMakeFiles/jp_grpc_proto.dir/jpeg.pb.cc.s

# Object files for target jp_grpc_proto
jp_grpc_proto_OBJECTS = \
"CMakeFiles/jp_grpc_proto.dir/jpeg.grpc.pb.cc.o" \
"CMakeFiles/jp_grpc_proto.dir/jpeg.pb.cc.o"

# External object files for target jp_grpc_proto
jp_grpc_proto_EXTERNAL_OBJECTS =

libjp_grpc_proto.a: CMakeFiles/jp_grpc_proto.dir/jpeg.grpc.pb.cc.o
libjp_grpc_proto.a: CMakeFiles/jp_grpc_proto.dir/jpeg.pb.cc.o
libjp_grpc_proto.a: CMakeFiles/jp_grpc_proto.dir/build.make
libjp_grpc_proto.a: CMakeFiles/jp_grpc_proto.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/juno/grpc/examples/cpp/jpeg/cmake/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX static library libjp_grpc_proto.a"
	$(CMAKE_COMMAND) -P CMakeFiles/jp_grpc_proto.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/jp_grpc_proto.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/jp_grpc_proto.dir/build: libjp_grpc_proto.a

.PHONY : CMakeFiles/jp_grpc_proto.dir/build

CMakeFiles/jp_grpc_proto.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/jp_grpc_proto.dir/cmake_clean.cmake
.PHONY : CMakeFiles/jp_grpc_proto.dir/clean

CMakeFiles/jp_grpc_proto.dir/depend: jpeg.pb.cc
CMakeFiles/jp_grpc_proto.dir/depend: jpeg.pb.h
CMakeFiles/jp_grpc_proto.dir/depend: jpeg.grpc.pb.cc
CMakeFiles/jp_grpc_proto.dir/depend: jpeg.grpc.pb.h
	cd /home/juno/grpc/examples/cpp/jpeg/cmake/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/juno/grpc/examples/cpp/jpeg /home/juno/grpc/examples/cpp/jpeg /home/juno/grpc/examples/cpp/jpeg/cmake/build /home/juno/grpc/examples/cpp/jpeg/cmake/build /home/juno/grpc/examples/cpp/jpeg/cmake/build/CMakeFiles/jp_grpc_proto.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/jp_grpc_proto.dir/depend

