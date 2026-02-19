###############################################################################
# Dockerfile -- Multi-stage Build untuk IoT Bridge Application
#
# File ini mendefinisikan cara membangun Docker image untuk aplikasi.
# Menggunakan multi-stage build:
#   Stage 1 (builder) : Install semua tools, compile source code
#   Stage 2 (runtime) : Image kecil, hanya berisi binary dan library
#
# Keuntungan multi-stage:
#   - Image runtime jauh lebih kecil (tidak ada compiler, source code, dll)
#   - Lebih aman (mengurangi attack surface di production)
#
# Cara build:
#   docker build -t iot-bridge-app:v1.0 .
#
# Cara run:
#   docker run --env-file .env -p 50051:50051 -p 9002:9002 iot-bridge-app:v1.0
###############################################################################

# ===== BUILD STAGE =====
# Base image: Ubuntu 22.04 sebagai builder (berisi compiler dan tools)
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive  
# noninteractive: agar apt-get tidak bertanya interaktif saat build

# 1. Install build dependencies
# Semua package yang diperlukan untuk compile C++ project:
#   - build-essential, g++, gcc : compiler C/C++
#   - cmake : build system (diinstall manual di step 2 karena butuh versi baru)
#   - git, wget, curl : download dependencies
#   - libssl-dev : OpenSSL untuk gRPC TLS
#   - python3, pip : untuk Conan package manager
#   - autoconf, bison, flex, perl : untuk build OpenDDS dari source
RUN apt-get update -y && \
    apt-get install -y --no-install-recommends \
    autoconf \
    bison \
    build-essential \
    ca-certificates \
    curl \
    flex \
    g++ \
    gcc \
    git \
    libssl-dev \
    pkg-config \
    python3 \
    python3-pip \
    perl \
    tar \
    unzip \
    wget \
    zip \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

# 2. Install CMake 3.31.3
# Versi CMake dari apt terlalu lama, project butuh minimal 3.31.0
# Download binary pre-built dari GitHub dan link ke /usr/local/bin
ADD https://github.com/Kitware/CMake/releases/download/v3.31.3/cmake-3.31.3-linux-x86_64.tar.gz \
    cmake-3.31.3-linux-x86_64.tar.gz
RUN tar -xzf cmake-3.31.3-linux-x86_64.tar.gz && \
    mv cmake-3.31.3-linux-x86_64 /usr/local/cmake && \
    ln -s /usr/local/cmake/bin/* /usr/local/bin && \
    rm cmake-3.31.3-linux-x86_64.tar.gz

# 3. Build OpenDDS 3.29.1 from source
# OpenDDS tidak tersedia di Conan, harus di-build manual.
# Proses build cukup lama (15-30 menit tergantung mesin).
# Hasil build: libraries (.so) dan tools (tao_idl, opendds_idl)
WORKDIR /opt
RUN wget https://github.com/OpenDDS/OpenDDS/releases/download/DDS-3.29.1/OpenDDS-3.29.1.tar.gz -O OpenDDS.tar.gz && \
    tar -xzf OpenDDS.tar.gz && \
    mv OpenDDS-3.29.1 OpenDDS && \
    rm OpenDDS.tar.gz

WORKDIR /opt/OpenDDS
RUN ./configure && \
    make -j$(nproc)

# Environment variables untuk OpenDDS, ACE, dan TAO
# DDS_ROOT/OPENDDS_HOME : lokasi OpenDDS
# ACE_ROOT : lokasi ACE framework (base library OpenDDS)
# TAO_ROOT : lokasi TAO CORBA ORB (dipakai OpenDDS internal)
# MPC_ROOT : lokasi MPC build system (Makefile Project Creator)
# LD_LIBRARY_PATH : path ke shared libraries (.so) agar bisa ditemukan saat runtime
ENV DDS_ROOT=/opt/OpenDDS \
    OPENDDS_HOME=/opt/OpenDDS \
    ACE_ROOT=/opt/OpenDDS/ACE_wrappers \
    TAO_ROOT=/opt/OpenDDS/ACE_wrappers/TAO \
    MPC_ROOT=/opt/OpenDDS/MPC \
    LD_LIBRARY_PATH=/opt/OpenDDS/lib:/opt/OpenDDS/ACE_wrappers/lib \
    PATH=/opt/OpenDDS/bin:/opt/OpenDDS/ACE_wrappers/bin:/usr/local/cmake/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

# 4. Setup Conan package manager
# Conan digunakan untuk menginstall dependency C++ (gRPC, protobuf, spdlog, dll)
# "conan profile detect" membuat profil default berdasarkan compiler yang terinstall
RUN pip3 install --no-cache-dir conan && conan profile detect --force

WORKDIR /app

# 5. Install Conan dependencies (layer cached jika conanfile.txt tidak berubah)
# Docker cache: jika conanfile.txt sama, step ini di-skip (hemat waktu build)
COPY conanfile.txt .
RUN conan install . --output-folder=build --build=missing

# 6. Copy source code dan konfigurasi ke dalam container
# Docker-specific CMakeLists.txt di-copy terpisah karena path mungkin berbeda
COPY ./docker/CMakeLists.txt /app/CMakeLists.txt
COPY ./CMakeUserPresets.json /app/CMakeUserPresets.json
# Source code C++
COPY ./src /app/src      
# File .proto untuk gRPC
COPY ./proto /app/proto  
# File .idl untuk OpenDDS
COPY ./idl /app/idl      
# Konfigurasi RTPS (DDS transport)
COPY ./rtps.ini /app/    

# 6b. Regenerate IDL TypeSupport files dengan TAO_IDL versi Docker
# PENTING: versi TAO_IDL harus cocok dengan versi OpenDDS yang di-build.
# File IDL yang di-generate di mesin lokal mungkin tidak kompatibel
# dengan versi OpenDDS di Docker, jadi harus di-generate ulang di sini.
RUN cd /app/idl/SensorData && \
    tao_idl -I "$DDS_ROOT" -I . -Sa -St -Sm -Sci -in --idl-version 4 --unknown-annotations ignore SensorData.idl && \
    opendds_idl -I . -Gxtypes-complete SensorData.idl && \
    tao_idl -I "$DDS_ROOT" -I . -Sa -St -Sm -Sci -in --idl-version 4 --unknown-annotations ignore SensorDataTypeSupport.idl

# 7. Build project menggunakan CMake
# cmake --preset : gunakan preset dari CMakePresets.json (konfigurasi Conan)
# cmake --build  : compile source code menjadi binary
# -j$(nproc)     : parallel build menggunakan semua CPU core
RUN cmake --preset conan-release && \
    cmake --build build --config Release -j$(nproc)

# 7b. Kumpulkan semua shared library OpenDDS ke satu folder
# Masalah: library .so tersebar di banyak subfolder OpenDDS build tree
# Solusi: copy semua .so ke satu folder flat, buat symlink yang diperlukan
# Ini memudahkan COPY ke runtime stage
RUN mkdir -p /opt/opendds-runtime-libs && \
    find /opt/OpenDDS -name "*.so*" ! -type l -exec cp {} /opt/opendds-runtime-libs/ \; && \
    cd /opt/opendds-runtime-libs && \
    for f in *.so.*; do \
    base=$(echo "$f" | sed 's/\.so\..*/\.so/'); \
    [ ! -e "$base" ] && ln -s "$f" "$base" || true; \
    done

# ===== RUNTIME STAGE =====
# Image runtime: Ubuntu 22.04 minimal (tanpa compiler/tools)
# Hanya berisi binary aplikasi dan library yang diperlukan
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install hanya runtime dependencies yang diperlukan:
#   - libstdc++6    : C++ standard library (wajib untuk binary C++)
#   - ca-certificates: sertifikat SSL
#   - libssl3       : OpenSSL runtime
RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 \
    ca-certificates \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

# Buat direktori untuk logs
RUN mkdir -p /app/build/logs
WORKDIR /app/build

# Copy binary hasil build dari builder stage
COPY --from=builder /app/build/iot_bridge /app/build/iot_bridge
# Copy konfigurasi RTPS untuk DDS
COPY --from=builder /app/rtps.ini /app/build/rtps.ini

# Copy OpenDDS runtime libraries (sudah di-flatten di step 7b)
COPY --from=builder /opt/opendds-runtime-libs /opt/opendds-libs

# Daftarkan path library ke dynamic linker
# Agar binary bisa menemukan .so files saat runtime
RUN echo "/opt/opendds-libs" > /etc/ld.so.conf.d/opendds.conf && \
    ldconfig

# Runtime environment variables
ENV LD_LIBRARY_PATH="/opt/opendds-libs" \
    PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

# Non-root user untuk keamanan
# Menjalankan container sebagai root adalah security risk
# iotuser memiliki akses minimal yang diperlukan
RUN useradd -ms /bin/bash iotuser && \
    chown iotuser:iotuser -R /app && \
    chmod -R 500 /app && \             
    chmod -R 700 /app/build/logs       
# 500 = read+execute (binary bisa dijalankan tapi tidak diubah)
# 700 = read+write+execute (logs bisa ditulis)

USER iotuser

# Command default saat container dijalankan
# Menjalankan binary iot_bridge yang sudah di-compile
CMD ["./iot_bridge"]