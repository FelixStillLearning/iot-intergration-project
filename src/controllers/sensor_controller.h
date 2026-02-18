#pragma once
#include "sensor.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>
#include <memory>

class BridgeManager;

class SensorController final : public iot::SensorService::Service {
public:
    explicit SensorController(std::shared_ptr<BridgeManager> bridge);

    grpc::Status SendSensorData(grpc::ServerContext* context, 
                               const iot::SensorRequest* request, 
                               iot::SensorResponse* response) override;

    grpc::Status StreamSensorData(grpc::ServerContext* context,
                                 grpc::ServerReader<iot::SensorRequest>* reader,
                                 iot::SensorResponse* response) override;

    grpc::Status MonitorSensor(grpc::ServerContext* context,
                              const iot::SensorRequest* request,
                              grpc::ServerWriter<iot::SensorResponse>* writer) override;

    grpc::Status InteractiveSensor(grpc::ServerContext* context,
                                  grpc::ServerReaderWriter<iot::SensorResponse, iot::SensorRequest>* stream) override;

private:
    std::shared_ptr<BridgeManager> bridge_;
};
