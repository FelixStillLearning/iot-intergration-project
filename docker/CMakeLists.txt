cmake_minimum_required(VERSION 3.31.0)
project(iot-integration-project VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

include(CTest)
enable_testing()

find_package(Threads REQUIRED)
find_package(absl CONFIG REQUIRED)             
find_package(Boost REQUIRED COMPONENTS thread)  
find_package(spdlog CONFIG REQUIRED) 
find_package(fmt REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(websocketpp REQUIRED)             
find_package(libuv CONFIG REQUIRED) 
find_package(absl CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread)          
find_package(uwebsockets CONFIG REQUIRED)
find_package(usockets CONFIG REQUIRED)
find_package(ZLIB REQUIRED) 


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


file(GLOB_RECURSE SOURCES src/*.cpp src/*.cc src/*.h src/*.hpp)

list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/app.cpp)

add_library(iot-core-objects OBJECT ${SOURCES} ${PROTO_GEN_FILES})

include_directories(src)
include_directories(${PROTO_OUT_DIR})
include_directories(${absl_INCLUDE_DIR})   
include_directories(${Boost_INCLUDE_DIR})  
include_directories(${libuv_INCLUDE_DIR})  
include_directories(${spdlog_INCLUDE_DIR}) 
include_directories(${fmt_INCLUDE_DIR})
include_directories(
    "$ENV{OPENDDS_HOME}"
    "$ENV{ACE_ROOT}"
    "$ENV{TAO_ROOT}"
)
link_directories(
    "$ENV{OPENDDS_HOME}/lib"
    "$ENV{ACE_ROOT}/lib"
)
include_directories(
    ${uwebsockets_INCLUDE_DIR}
    ${usockets_INCLUDE_DIR}
)

add_executable(iot_bridge src/app.cpp $<TARGET_OBJECTS:iot-core-objects>)

target_link_libraries(iot_bridge PRIVATE

    Threads::Threads
    abseil::abseil              
    Boost::boost 
    Boost::thread  
    spdlog::spdlog 
    fmt::fmt
    rapidjson

    OpenDDS_Dcps
    TAO_ValueType
    TAO
    ACE

    gRPC::grpc++ gRPC::grpc++_alts 
    protobuf::libprotobuf
    websocketpp::websocketpp
    libuv::libuv 
    abseil::abseil               
    usockets::usockets   
    ZLIB::ZLIB           
    uv                   
    z                   
)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)