# Use base image (Ubuntu 22.04) as BUILD STAGE
FROM ubuntu:22.04 AS build

ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies with minimal packages
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
        tar \
        unzip \
        wget \
        zip \
        zlib1g-dev && \
    rm -rf /var/lib/apt/lists/*

# Install CMake using precompiled binary
ADD https://github.com/Kitware/CMake/releases/download/v3.31.3/cmake-3.31.3-linux-x86_64.tar.gz \
        cmake-3.31.3-linux-x86_64.tar.gz
RUN tar -xzf cmake-3.31.3-linux-x86_64.tar.gz && \
    mv cmake-3.31.3-linux-x86_64 /usr/local/cmake && \
    ln -s /usr/local/cmake/bin/* /usr/local/bin && \
    rm cmake-3.31.3-linux-x86_64.tar.gz

# Set a dedicated working directory for your application
WORKDIR /app

# Copy only configuration files first to leverage Docker layer caching
COPY ./docker/CMakeLists.txt /app/CMakeLists.txt
COPY ./conanfile.txt /app/conanfile.txt

# Install and configure Conan (this layer is cached if these files don’t change)
RUN pip3 install --no-cache-dir conan && \
    conan profile detect --force && \
    conan install . --output-folder=build --build=missing

# Configure the project using CMake presets
RUN cmake --preset conan-release

# Build the project using parallelism
RUN cmake --build "/app/build" --config Release --target all -j$(nproc)

# Start a new stage, this stage will run programs from the build stage
FROM ubuntu:22.04

# Create the /app directory
RUN mkdir -p /app/build/logs

WORKDIR /app/build

# Copy files from the build stage
COPY --from=build /app/build/simple-grpc-cpp /app/build/simple-grpc-cpp

# Set args
ARG GRPC_PORT

# Set exposed ports
EXPOSE ${GRPC_PORT}

# Create a non-root user for running the application and set permissions to the application directory
RUN useradd -ms /bin/bash serviceuser && \
    chown serviceuser:serviceuser -R /app && \
    chmod -R 500 /app && \
    chmod -R 700 /app/build/logs

# Switch to the non-root user for improved security
USER serviceuser

# Run the app
CMD ["./simple-grpc-cpp"]
