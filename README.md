# IoT Integration Project

Simple bridge that accepts gRPC sensor data and relays it to WebSocket (JSON) and DDS.

## Requirements
- C++17 toolchain
- CMake 3.31+
- Conan
- OpenDDS installed and configured
- gRPC + Protobuf
- WebSocketPP

## Environment
Copy and edit the env file:
```
cp .env.example .env
```

Load env before running:
```
export $(grep -v '^#' .env | xargs)
```

Note:
- Jangan gunakan `setenv.sh` jika path di dalamnya tidak sesuai. Gunakan `.env` dengan path OpenDDS yang benar.

## Build
```
rm -rf build
conan install . --output-folder=build --build=missing
cmake --preset conan-release
cmake --build build -j
```

## Run
```
./build/iot_bridge
```

## Run with Docker
1. Ensure `.env` is filled (required for ports and runtime config).
2. Build and start containers:
```
docker compose up --build
```
3. Stop containers:
```
docker compose down
```

Notes:
- Ports are mapped from `.env` (`GRPC_PORT`, `WS_PORT`, `INFO_REPO_PORT`).
- If `.env` is missing or incomplete, Docker Compose will fail.

## Testing

### 1) BloomRPC (gRPC)
1. Open BloomRPC.
2. Import proto: `proto/sensor.proto`.
3. Set server: `GRPC_HOST:GRPC_PORT` (default `0.0.0.0:50051`).
4. Call `SendSensorData` with sample payload:
```
{
  "sensor_id": 1,
  "sensor_name": "sensor-1",
  "temperature": 25.5,
  "humidity": 60.2,
  "pressure": 1013.2,
  "light_intensity": 300.5,
  "timestamp": 1700000000,
  "location": "lab"
}
```

### 2) Postman (WebSocket)
1. Open Postman.
2. Create a WebSocket request to `ws://localhost:WS_PORT` (default `9002`).
3. Connect and observe broadcast messages in JSON.

### 3) OpenDDS Monitor
1. Start OpenDDS Monitor.
2. Connect to your DDS domain (default `DDS_DOMAIN=0`).
3. Subscribe to the topic configured in `.env` (`TEST_TOPIC`).
4. Send gRPC requests and verify DDS data is published.

## Optional CLI Test (grpcurl)
If you have grpcurl installed:
```
grpcurl -plaintext -d '{"sensor_id":1,"sensor_name":"sensor-1","temperature":25.5,"humidity":60.2,"pressure":1013.2,"light_intensity":300.5,"timestamp":1700000000,"location":"lab"}' \
  ${GRPC_HOST:-0.0.0.0}:${GRPC_PORT:-50051} iot.SensorService/SendSensorData
```
