cmake_minimum_required(VERSION 3.24)
project(MessengerTests C CXX)
set(project_prefix "messenger")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(GTest REQUIRED)

include(../common.cmake)

# Proto file
get_filename_component(ms_proto "../protos/${project_prefix}.proto" ABSOLUTE)
get_filename_component(ms_proto_path "${ms_proto}" PATH)

# Generated sources
set(ms_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/${project_prefix}.pb.cc")
set(ms_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/${project_prefix}.pb.h")
set(ms_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/${project_prefix}.grpc.pb.cc")
set(ms_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/${project_prefix}.grpc.pb.h")
add_custom_command(
      OUTPUT  "${ms_proto_srcs}" "${ms_proto_hdrs}" "${ms_grpc_srcs}" "${ms_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${ms_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${ms_proto}"
      DEPENDS "${ms_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# ms_grpc_proto
add_library(ms_grpc_proto
  ${ms_grpc_srcs}
  ${ms_grpc_hdrs}
  ${ms_proto_srcs}
  ${ms_proto_hdrs})
target_link_libraries(ms_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

add_executable(test_db test_db.cc ../server/database/credentials_db.cc)

# Targets
foreach(_target
  test_db)
  target_link_libraries(${_target}
    ms_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    ${GTEST_LIBRARIES})
endforeach()

# add_executable(Messenger main.cc queue.h net/net_service.cc net/net_service.h message.cc message.h message_visitor.h interface/interface_service.cc interface/interface_service.h)

