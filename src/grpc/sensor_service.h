#pragma once
#include "sensor.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>

class SensorServiceImpl final : public iot::SensorService::Service {
public:
    // 1. Unary
    grpc::Status SendSensorData(grpc::ServerContext* context, 
                               const iot::SensorRequest* request, 
                               iot::SensorResponse* response) override;

    // 2. Client Streaming
    grpc::Status StreamSensorData(grpc::ServerContext* context,
                                 grpc::ServerReader<iot::SensorRequest>* reader,
                                 iot::SensorResponse* response) override;

    // 3. Server Streaming
    grpc::Status MonitorSensor(grpc::ServerContext* context,
                              const iot::SensorRequest* request,
                              grpc::ServerWriter<iot::SensorResponse>* writer) override;

    // 4. Bidirectional Streaming
    grpc::Status InteractiveSensor(grpc::ServerContext* context,
                                  grpc::ServerReaderWriter<iot::SensorResponse, iot::SensorRequest>* stream) override;
};