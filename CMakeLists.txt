###############################################################################
# CMakeLists.txt -- Build Configuration untuk IoT Integration Project
#
# File ini mendefinisikan bagaimana project di-build menggunakan CMake.
# CMake adalah build system generator yang membuat Makefile/Ninja files
# berdasarkan konfigurasi di file ini.
#
# Komponen utama yang di-build:
#   1. iot-core-objects : Library object berisi semua source code kecuali main
#   2. iot_bridge       : Executable utama (link iot-core-objects + main)
#
# Dependencies eksternal (diinstall via Conan package manager):
#   - gRPC + Protobuf  : komunikasi RPC antar-service
#   - OpenDDS          : middleware DDS untuk publish-subscribe
#   - websocketpp      : WebSocket server library
#   - spdlog + fmt     : logging library
#   - RapidJSON        : JSON serialization
#   - Boost            : utility library (thread, dll)
#   - ASIO (via libuv) : asynchronous I/O
#
# Cara build:
#   1. Install dependencies: conan install . --output-folder=build
#   2. Configure: cmake --preset conan-release
#   3. Build: cmake --build build
###############################################################################

# Versi minimum CMake yang diperlukan
cmake_minimum_required(VERSION 3.31.0)

# Nama project, versi, dan bahasa yang digunakan
project(iot-integration-project VERSION 0.1.0 LANGUAGES C CXX)

# Export compile_commands.json untuk IDE (autocomplete, error checking)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Gunakan C++17 standard (diperlukan oleh gRPC, spdlog, dll)
set(CMAKE_CXX_STANDARD 17)

# Aktifkan CTest untuk unit testing
include(CTest)
enable_testing()

###############################################################################
# Cari semua dependency yang dibutuhkan
# find_package() mencari library yang sudah terinstall (via Conan atau system)
###############################################################################
find_package(Threads REQUIRED)          # POSIX threads (std::thread)
find_package(absl CONFIG REQUIRED)      # Abseil (utility library dari Google)
find_package(Boost REQUIRED COMPONENTS thread)  # Boost.Thread
find_package(spdlog CONFIG REQUIRED)    # Logging library
find_package(fmt REQUIRED)              # String formatting (dipakai spdlog)
find_package(RapidJSON REQUIRED)        # JSON parser/generator
find_package(protobuf CONFIG REQUIRED)  # Protocol Buffers (serialization)
find_package(gRPC CONFIG REQUIRED)      # gRPC framework
find_package(websocketpp REQUIRED)      # WebSocket server library
find_package(libuv CONFIG REQUIRED)     # Async I/O library
find_package(absl CONFIG REQUIRED)      # (duplikat, tidak masalah)
find_package(Boost REQUIRED COMPONENTS thread)  # (duplikat, tidak masalah)
find_package(uwebsockets CONFIG REQUIRED) # uWebSockets library
find_package(usockets CONFIG REQUIRED)    # uSockets (backend uWebSockets)
find_package(ZLIB REQUIRED)             # Compression library


###############################################################################
# Protobuf dan gRPC Code Generation
# 
# Dari file sensor.proto, generate 4 file C++:
#   - sensor.pb.h/.cc       : Protobuf message classes (SensorRequest, SensorResponse)
#   - sensor.grpc.pb.h/.cc  : gRPC service stubs (SensorService)
#
# protoc (protobuf compiler) dijalankan saat build dengan plugin grpc_cpp_plugin
###############################################################################
set(PROTO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(PROTO_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_OUT_DIR})  # Buat folder output jika belum ada

# Daftar file yang akan di-generate dari sensor.proto
set(PROTO_GEN_FILES
    "${PROTO_OUT_DIR}/sensor.pb.cc"       # Protobuf message implementation
    "${PROTO_OUT_DIR}/sensor.pb.h"        # Protobuf message header
    "${PROTO_OUT_DIR}/sensor.grpc.pb.cc"  # gRPC service implementation
    "${PROTO_OUT_DIR}/sensor.grpc.pb.h"   # gRPC service header
)

# Custom command: jalankan protoc untuk generate code dari .proto
# Command ini hanya dijalankan ulang jika sensor.proto berubah
add_custom_command(
    OUTPUT ${PROTO_GEN_FILES}
    COMMAND protobuf::protoc --cpp_out=${PROTO_OUT_DIR}    # Generate protobuf C++
            --grpc_out=${PROTO_OUT_DIR}                     # Generate gRPC C++
            --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>  # Plugin gRPC
            -I ${PROTO_SRC_DIR} ${PROTO_SRC_DIR}/sensor.proto  # Input file
    DEPENDS ${PROTO_SRC_DIR}/sensor.proto  # Dependency: hanya rebuild jika .proto berubah
    COMMENT "Generating gRPC and Protobuf C++ files from sensor.proto"
)

###############################################################################
# OpenDDS IDL Generated Sources
#
# File-file ini di-generate dari SensorData.idl menggunakan script ./generate_idl
# IDL (Interface Definition Language) mendefinisikan struktur data untuk DDS.
# Berbeda dengan protobuf, IDL files harus di-generate SEBELUM build.
#
# File yang di-generate:
#   - SensorDataC.cpp       : Client-side type support
#   - SensorDataS.cpp       : Server-side type support
#   - SensorDataTypeSupport* : Type registration dan serialization
###############################################################################
set(IDL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/idl/SensorData)
set(IDL_GEN_FILES
    "${IDL_DIR}/SensorDataC.cpp"              # IDL Client stub
    "${IDL_DIR}/SensorDataS.cpp"              # IDL Server skeleton
    "${IDL_DIR}/SensorDataTypeSupportC.cpp"    # Type support client
    "${IDL_DIR}/SensorDataTypeSupportS.cpp"    # Type support server
    "${IDL_DIR}/SensorDataTypeSupportImpl.cpp" # Type support implementation
)

###############################################################################
# Kumpulkan semua source files dari folder src/
# GLOB_RECURSE mencari secara rekursif semua file .cpp, .cc, .h, .hpp
###############################################################################
file(GLOB_RECURSE SOURCES src/*.cpp src/*.cc src/*.h src/*.hpp)

# Keluarkan app.cpp dari library (karena berisi main(), harus di executable)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/app.cpp)

# Keluarkan file-file yang sedang di-refactor (belum siap)
file(GLOB_RECURSE NEED_TO_REFACTOR src/need_to_refactor/*)
list(REMOVE_ITEM SOURCES ${NEED_TO_REFACTOR})

###############################################################################
# iot-core-objects -- Object Library
#
# Berisi semua source code KECUALI main().
# Dibuat sebagai OBJECT library agar bisa di-share antara:
#   - executable utama (iot_bridge)
#   - unit tests (jika ada)
# Tanpa ini, source harus di-compile ulang untuk setiap target.
###############################################################################
add_library(iot-core-objects OBJECT ${SOURCES} ${PROTO_GEN_FILES} ${IDL_GEN_FILES})

# Link dependencies ke object library
# PRIVATE berarti dependency hanya digunakan saat compile, tidak di-propagate
target_link_libraries(iot-core-objects PRIVATE
    Threads::Threads          # std::thread, std::mutex
    abseil::abseil            # Google Abseil utilities
    Boost::boost              # Boost headers
    Boost::thread             # Boost.Thread
    spdlog::spdlog            # Logging
    fmt::fmt                  # String formatting
    rapidjson                 # JSON parser

    OpenDDS_Dcps              # OpenDDS core library
    TAO_Valuetype             # TAO CORBA valuetype support
    TAO                       # TAO CORBA ORB
    ACE                       # Adaptive Communication Environment (base TAO)

    gRPC::grpc++ gRPC::grpc++_alts  # gRPC C++ library
    protobuf::libprotobuf     # Protobuf runtime
    websocketpp::websocketpp  # WebSocket library
    libuv::uv_a               # Async I/O (static)
    abseil::abseil            # (duplikat link, tidak masalah)
    usockets::usockets        # uSockets library
    ZLIB::ZLIB                # zlib compression
    uv                        # libuv (dynamic)
    z                         # zlib (dynamic)
)

###############################################################################
# Include Directories
# Tambahkan folder-folder yang berisi header files agar compiler bisa
# menemukan #include tanpa path absolut.
###############################################################################
include_directories(src)               # Source code project
include_directories(${PROTO_OUT_DIR})  # Generated protobuf/gRPC headers
include_directories(${IDL_DIR})        # Generated OpenDDS IDL headers
include_directories(${absl_INCLUDE_DIR})    # Abseil headers
include_directories(${Boost_INCLUDE_DIR})   # Boost headers
include_directories(${libuv_INCLUDE_DIR})   # libuv headers
include_directories(${spdlog_INCLUDE_DIR})  # spdlog headers
include_directories(${fmt_INCLUDE_DIR})     # fmt headers

# OpenDDS/TAO/ACE headers (dari environment variables)
# Hanya include jika environment variable tidak kosong
if(DEFINED ENV{OPENDDS_HOME} AND NOT "$ENV{OPENDDS_HOME}" STREQUAL "")
    include_directories("$ENV{OPENDDS_HOME}")
endif()

if(DEFINED ENV{ACE_ROOT} AND NOT "$ENV{ACE_ROOT}" STREQUAL "")
    include_directories("$ENV{ACE_ROOT}")
endif()

if(DEFINED ENV{TAO_ROOT} AND NOT "$ENV{TAO_ROOT}" STREQUAL "")
    include_directories("$ENV{TAO_ROOT}")
endif()

# Link directories untuk OpenDDS/ACE libraries
if(DEFINED ENV{OPENDDS_HOME} AND NOT "$ENV{OPENDDS_HOME}" STREQUAL "")
    link_directories("$ENV{OPENDDS_HOME}/lib")
endif()

if(DEFINED ENV{ACE_ROOT} AND NOT "$ENV{ACE_ROOT}" STREQUAL "")
    link_directories("$ENV{ACE_ROOT}/lib")
endif()

# uWebSockets dan uSockets headers
include_directories(
    ${uwebsockets_INCLUDE_DIR}  # uWebSockets headers
    ${usockets_INCLUDE_DIR}     # uSockets headers
)

###############################################################################
# iot_bridge -- Executable Utama
#
# Ini adalah binary yang dijalankan saat aplikasi start.
# Menggabungkan app.cpp (berisi main()) dengan iot-core-objects.
#
# Build menghasilkan file: build/iot_bridge
# Jalankan: ./build/iot_bridge
###############################################################################
add_executable(iot_bridge src/app.cpp $<TARGET_OBJECTS:iot-core-objects>)

# Link dependencies ke executable
# Sama dengan iot-core-objects karena executable juga membutuhkan libraries ini
target_link_libraries(iot_bridge PRIVATE

    Threads::Threads          # Threading support
    abseil::abseil            # Google Abseil
    Boost::boost              # Boost headers
    Boost::thread             # Boost.Thread
    spdlog::spdlog            # Logging
    fmt::fmt                  # String formatting
    rapidjson                 # JSON

    OpenDDS_Dcps              # DDS core
    TAO_Valuetype             # CORBA valuetype
    TAO                       # CORBA ORB
    ACE                       # ACE framework

    gRPC::grpc++ gRPC::grpc++_alts  # gRPC
    protobuf::libprotobuf     # Protobuf
    websocketpp::websocketpp  # WebSocket
    libuv::uv_a               # Async I/O
    abseil::abseil            # Abseil (duplikat)
    usockets::usockets        # uSockets
    ZLIB::ZLIB                # Compression
    uv                        # libuv
    z                         # zlib
)


###############################################################################
# CPack Configuration -- Packaging
# Digunakan untuk membuat distribusi package (tar.gz, deb, rpm, dll)
# Jalankan: cpack --config build/CPackConfig.cmake
###############################################################################
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)