cmake_minimum_required(VERSION 3.8)

project(remote C CXX)

include(common.cmake)

find_package(OpenCV REQUIRED)

# Proto file
get_filename_component(remote_message_proto "./protos/remote_message.proto" ABSOLUTE)
get_filename_component(remote_message_proto_path "${remote_message_proto}" PATH)

# Generated sources
set(remote_message_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/remote_message.pb.cc")
set(remote_message_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/remote_message.pb.h")
set(remote_message_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/remote_message.grpc.pb.cc")
set(remote_message_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/remote_message.grpc.pb.h")
add_custom_command(
      OUTPUT "${remote_message_proto_srcs}" "${remote_message_proto_hdrs}" "${remote_message_grpc_srcs}" "${remote_message_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS  --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
            --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
            -I "${remote_message_proto_path}"
            --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
            "${remote_message_proto}"
      DEPENDS "${remote_message_proto}"
      COMMENT "Generating gRPC and Protobuf code from ${remote_message_proto}")

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
  add_executable(${_target} 
    "${_target}.cc" 
    "media_handler.cc" "media_handler.h") 
  target_link_libraries(
    ${_target}
    remote_message_grpc_proto
    absl::check
    absl::flags
    absl::flags_parse
    absl::log
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    ${OpenCV_LIBS})
endforeach()
