# STAGE 1: The Build
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# 1. Install dependencies dasar
RUN apt-get update && apt-get install -y \
    build-essential \
    python3-pip \
    git \
    wget \
    pkg-config \
    apt-transport-https \
    ca-certificates \
    gnupg \
    lsb-release \
    && rm -rf /var/lib/apt/lists/*

# 2. Ambil CMake terbaru dari Kitware (Sesuai kebutuhan v3.31.0)
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor -o /usr/share/keyrings/kitware-archive-keyring.gpg \
    && echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" | tee /etc/apt/sources.list.d/kitware.list >/dev/null \
    && apt-get update \
    && apt-get install -y cmake \
    && rm -rf /var/lib/apt/lists/*

# 3. Setup Conan
RUN pip3 install conan && conan profile detect --force

WORKDIR /app

# 4. Install library 
COPY conanfile.txt .
RUN conan install . --output-folder=build --build=missing

# 5. Copy source code dan rakit biner
COPY . .
RUN cmake -S . -B build \
    -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --config Release

# STAGE 2: The Runtime Environment
FROM ubuntu:22.04


RUN apt-get update && apt-get install -y \
    libstdc++6 \
    ca-certificates \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/iot_bridge .

RUN useradd -ms /bin/bash iotuser
USER iotuser

# Ekspos port gRPC (50051) dan WebSocket (9002)
EXPOSE 50051 9002

CMD ["./iot_bridge"]