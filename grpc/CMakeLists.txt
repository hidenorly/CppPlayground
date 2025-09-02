cmake_minimum_required(VERSION 3.14)
project(grpc_example CXX)

set(CMAKE_CXX_STANDARD 20)

#find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/example.proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

add_custom_command(
  OUTPUT ${GENERATED_DIR}/example.pb.cc ${GENERATED_DIR}/example.pb.h
         ${GENERATED_DIR}/example.grpc.pb.cc ${GENERATED_DIR}/example.grpc.pb.h
  COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
    --cpp_out=${GENERATED_DIR}
    --grpc_out=${GENERATED_DIR}
    #--plugin=protoc-gen-grpc=$<TARGET_FILE:grpc_cpp_plugin>
    --plugin=protoc-gen-grpc=/opt/homebrew/bin/grpc_cpp_plugin
    ${PROTO_FILE}
  DEPENDS ${PROTO_FILE}
)

add_library(example_proto
  ${GENERATED_DIR}/example.pb.cc
  ${GENERATED_DIR}/example.grpc.pb.cc
)
target_include_directories(example_proto PUBLIC ${GENERATED_DIR})
target_link_libraries(example_proto PUBLIC gRPC::grpc++ protobuf::libprotobuf)

add_executable(ExampleServer ExampleService.cxx)
target_link_libraries(ExampleServer PRIVATE example_proto gRPC::grpc++ protobuf::libprotobuf)

#add_executable(ExampleClient ExampleClient.cxx)
#target_link_libraries(ExampleClient PRIVATE example_proto gRPC::grpc++ protobuf::libprotobuf)
