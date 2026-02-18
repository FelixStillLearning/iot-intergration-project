# ===== BUILD STAGE =====
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# 1. Install build dependencies
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
ADD https://github.com/Kitware/CMake/releases/download/v3.31.3/cmake-3.31.3-linux-x86_64.tar.gz \
        cmake-3.31.3-linux-x86_64.tar.gz
RUN tar -xzf cmake-3.31.3-linux-x86_64.tar.gz && \
    mv cmake-3.31.3-linux-x86_64 /usr/local/cmake && \
    ln -s /usr/local/cmake/bin/* /usr/local/bin && \
    rm cmake-3.31.3-linux-x86_64.tar.gz

# 3. Build OpenDDS 3.29.1 from source
WORKDIR /opt
RUN wget https://github.com/OpenDDS/OpenDDS/releases/download/DDS-3.29.1/OpenDDS-3.29.1.tar.gz -O OpenDDS.tar.gz && \
    tar -xzf OpenDDS.tar.gz && \
    mv OpenDDS-3.29.1 OpenDDS && \
    rm OpenDDS.tar.gz

WORKDIR /opt/OpenDDS
RUN ./configure && \
    make -j$(nproc)

ENV DDS_ROOT=/opt/OpenDDS \
    OPENDDS_HOME=/opt/OpenDDS \
    ACE_ROOT=/opt/OpenDDS/ACE_wrappers \
    TAO_ROOT=/opt/OpenDDS/ACE_wrappers/TAO \
    MPC_ROOT=/opt/OpenDDS/MPC \
    LD_LIBRARY_PATH=/opt/OpenDDS/lib:/opt/OpenDDS/ACE_wrappers/lib \
    PATH=/opt/OpenDDS/bin:/opt/OpenDDS/ACE_wrappers/bin:/usr/local/cmake/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

# 4. Setup Conan
RUN pip3 install --no-cache-dir conan && conan profile detect --force

WORKDIR /app

# 5. Install Conan dependencies (layer cached if conanfile.txt unchanged)
COPY conanfile.txt .
RUN conan install . --output-folder=build --build=missing

# 6. Copy Docker-specific CMakeLists, source code, and configs
COPY ./docker/CMakeLists.txt /app/CMakeLists.txt
COPY ./CMakeUserPresets.json /app/CMakeUserPresets.json
COPY ./src /app/src
COPY ./proto /app/proto
COPY ./idl /app/idl
COPY ./rtps.ini /app/

# 6b. Regenerate IDL TypeSupport files with Docker's TAO_IDL (version must match)
RUN cd /app/idl/SensorData && \
    tao_idl -I "$DDS_ROOT" -I . -Sa -St -Sm -Sci -in --idl-version 4 --unknown-annotations ignore SensorData.idl && \
    opendds_idl -I . -Gxtypes-complete SensorData.idl && \
    tao_idl -I "$DDS_ROOT" -I . -Sa -St -Sm -Sci -in --idl-version 4 --unknown-annotations ignore SensorDataTypeSupport.idl

# 7. Build the project
RUN cmake --preset conan-release && \
    cmake --build build --config Release -j$(nproc)

# 7b. Flatten ALL OpenDDS/ACE .so files into single directory for runtime
#     (actual files are scattered across build tree, lib/ only has symlinks)
RUN mkdir -p /opt/opendds-runtime-libs && \
    find /opt/OpenDDS -name "*.so*" ! -type l -exec cp {} /opt/opendds-runtime-libs/ \; && \
    cd /opt/opendds-runtime-libs && \
    for f in *.so.*; do \
        base=$(echo "$f" | sed 's/\.so\..*/\.so/'); \
        [ ! -e "$base" ] && ln -s "$f" "$base" || true; \
    done

# ===== RUNTIME STAGE =====
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 \
    ca-certificates \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /app/build/logs
WORKDIR /app/build

# Copy binary and config
COPY --from=builder /app/build/iot_bridge /app/build/iot_bridge
COPY --from=builder /app/rtps.ini /app/build/rtps.ini

# Copy OpenDDS runtime libraries (flattened, no symlinks)
COPY --from=builder /opt/opendds-runtime-libs /opt/opendds-libs

# Register shared libraries with the dynamic linker
RUN echo "/opt/opendds-libs" > /etc/ld.so.conf.d/opendds.conf && \
    ldconfig

# Runtime environment
ENV LD_LIBRARY_PATH="/opt/opendds-libs" \
    PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

# Non-root user
RUN useradd -ms /bin/bash iotuser && \
    chown iotuser:iotuser -R /app && \
    chmod -R 500 /app && \
    chmod -R 700 /app/build/logs

USER iotuser

CMD ["./iot_bridge"]