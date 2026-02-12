cmake_minimum_required(VERSION 3.31.0)
project(iot-integration-project VERSION 0.1.0 LANGUAGES C CXX)

# Pengaturan Standar C++ dan Export Command untuk IDE
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#cari library
find_package(Threads REQUIRED)
find_package(protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(spdlog REQUIRED) 
find_package(fmt REQUIRED)

#konfigurasi prtobuf dan grpc
set(PROTO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(PROTO_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_OUT_DIR})

set(PROTO_GEN_FILES
    "${PROTO_OUT_DIR}/sensor.pb.cc" 
    "${PROTO_OUT_DIR}/sensor.pb.h"
    "${PROTO_OUT_DIR}/sensor.grpc.pb.cc" 
    "${PROTO_OUT_DIR}/sensor.grpc.pb.h"
)

add_custom_command(
    OUTPUT ${PROTO_GEN_FILES}
    COMMAND protobuf::protoc --cpp_out=${PROTO_OUT_DIR} 
            --grpc_out=${PROTO_OUT_DIR} 
            --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
            -I ${PROTO_SRC_DIR} ${PROTO_SRC_DIR}/sensor.proto
    DEPENDS ${PROTO_SRC_DIR}/sensor.proto
    COMMENT "Generating gRPC and Protobuf C++ files from sensor.proto"
)

#MANAJEMEN SOURCE CODE 
file(GLOB_RECURSE APP_SOURCES "src/*.cpp")


add_executable(iot_bridge 
    ${APP_SOURCES} 
    ${PROTO_GEN_FILES}
)



target_link_libraries(iot_bridge PRIVATE
    gRPC::grpc++
    protobuf::libprotobuf
    Threads::Threads
    spdlog::spdlog 
    fmt::fmt
)

target_include_directories(iot_bridge PRIVATE 
    src/
    ${PROTO_OUT_DIR}
)

target_include_directories(iot_bridge SYSTEM PUBLIC ${PROTO_OUT_DIR})