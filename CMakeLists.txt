# # Copyright 2018 gRPC authors.
# #
# # Licensed under the Apache License, Version 2.0 (the "License");
# # you may not use this file except in compliance with the License.
# # You may obtain a copy of the License at
# #
# #     http://www.apache.org/licenses/LICENSE-2.0
# #
# # Unless required by applicable law or agreed to in writing, software
# # distributed under the License is distributed on an "AS IS" BASIS,
# # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# # See the License for the specific language governing permissions and
# # limitations under the License.
# #
# # cmake build file for C++ helloworld example.
# # Assumes protobuf and gRPC have been installed using cmake.
# # See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# # that automatically builds all the dependencies before building helloworld.

# cmake_minimum_required(VERSION 3.8)

# project(remote C CXX)

# include(../cmake/common.cmake)

# # Proto file
# get_filename_component(remote_message_proto "./protos/remote_message.proto" ABSOLUTE)
# get_filename_component(remote_message_proto_path "${remote_message_proto}" PATH)

# enable_language(CXX)
# set(CMAKE_CXX_STANDARD 17) # C++14...
# set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
# set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11


# # Generated sources
# set(remote_message_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/remote_message.pb.cc")
# set(remote_message_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/remote_message.pb.h")
# set(remote_message_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/remote_message.grpc.pb.cc")
# set(remote_message_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/remote_message.grpc.pb.h")
# add_custom_command(
#       OUTPUT "${remote_message_proto_srcs}" "${remote_message_proto_hdrs}" "${remote_message_grpc_srcs}" "${remote_message_grpc_hdrs}"
#       COMMAND ${_PROTOBUF_PROTOC}
#       ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
#         --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
#         -I "${remote_message_proto_path}"
#         --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
#         "${remote_message_proto}"
#       DEPENDS "${remote_message_proto}")

# # Include generated *.pb.h files
# include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# # remote_message_grpc_proto
# add_library(remote_message_grpc_proto
#   ${remote_message_grpc_srcs}
#   ${remote_message_grpc_hdrs}
#   ${remote_message_proto_srcs}
#   ${remote_message_proto_hdrs})
# target_link_libraries(remote_message_grpc_proto
#   absl::check
#   ${_REFLECTION}
#   ${_GRPC_GRPCPP}
#   ${_PROTOBUF_LIBPROTOBUF})

# # Targets greeter_[async_](client|server)
# foreach(_target
#   remote_client remote_server)
#   add_executable(${_target} "${_target}.cc")
#   target_link_libraries(${_target}
#     remote_message_grpc_proto
#     absl::check
#     absl::flags
#     absl::flags_parse
#     absl::log
#     ${_REFLECTION}
#     ${_GRPC_GRPCPP}
#     ${_PROTOBUF_LIBPROTOBUF})
# endforeach()
# Copyright 2018 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# cmake build file for C++ helloworld example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building helloworld.

cmake_minimum_required(VERSION 3.8)

project(remote C CXX)

include(../cmake/common.cmake)

# Proto file
get_filename_component(remote_message_proto "./protos/remote_message.proto" ABSOLUTE)
get_filename_component(remote_message_proto_path "${remote_message_proto}" PATH)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find libzip package
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBZIP REQUIRED libzip)

# Include libzip directories
include_directories(${LIBZIP_INCLUDE_DIRS})
link_directories(${LIBZIP_LIBRARY_DIRS})

# Generated sources
set(remote_message_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/remote_message.pb.cc")
set(remote_message_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/remote_message.pb.h")
set(remote_message_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/remote_message.grpc.pb.cc")
set(remote_message_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/remote_message.grpc.pb.h")
add_custom_command(
      OUTPUT "${remote_message_proto_srcs}" "${remote_message_proto_hdrs}" "${remote_message_grpc_srcs}" "${remote_message_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${remote_message_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${remote_message_proto}"
      DEPENDS "${remote_message_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# remote_message_grpc_proto
add_library(remote_message_grpc_proto
  ${remote_message_grpc_srcs}
  ${remote_message_grpc_hdrs}
  ${remote_message_proto_srcs}
  ${remote_message_proto_hdrs})
target_link_libraries(remote_message_grpc_proto
  absl::check
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# Targets remote_[client|server]
foreach(_target
  remote_client remote_server)
  add_executable(${_target} "${_target}.cc")
  target_link_libraries(${_target}
    remote_message_grpc_proto
    absl::check
    absl::flags
    absl::flags_parse
    absl::log
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    ${LIBZIP_LIBRARIES})
endforeach()

# Add zip_folder executable
add_executable(zip_folder zip_folder.cpp)
target_link_libraries(zip_folder ${LIBZIP_LIBRARIES})
