cmake_minimum_required(VERSION 3.31.0)
project(iot-integration-project VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

#CARI LIBRARY
find_package(threads REQUIRED)
find_package(protobuf REQUIRED)
find_package(gRPC REQUIRED)

#lOKASI FOLDER
set(PROTO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(PROTO_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_OUT_DIR})

add_custom_command(
    OUTPUT "${PROTO_OUT_DIR}/sensor.pb.cc" "${PROTO_OUT_DIR}/sensor.pb.h"
           "${PROTO_OUT_DIR}/sensor.grpc.pb.cc" "${PROTO_OUT_DIR}/sensor.grpc.pb.h"
    COMMAND protobuf::protoc --cpp_out=${PROTO_OUT_DIR} --grpc_out=${PROTO_OUT_DIR} 
            --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
            -I ${PROTO_SRC_DIR} ${PROTO_SRC_DIR}/sensor.proto
    DEPENDS ${PROTO_SRC_DIR}/sensor.proto
)

add_executable(iot_bridge 
    src/main.cpp 
    "${PROTO_OUT_DIR}/sensor.pb.cc" 
    "${PROTO_OUT_DIR}/sensor.grpc.pb.cc"
)

add_executable(dds_subscriber 
    src/subscriber.cpp
)

target_link_libraries(iot_bridge PRIVATE
    gRPC::grpc++
    protobuf::libprotobuf
    Threads::Threads
    
)

target_include_directories(iot_bridge PRIVATE ${PROTO_OUT_DIR} src/)