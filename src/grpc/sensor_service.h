#pragma once
#include "sensor.grpc.pb.h"
#include <grpcpp/grpcpp.h>

class SensorServiceImpl final : public iot::SensorService::Service {
public:
    grpc::Status SendSensorData(grpc::ServerContext* context, 
                               const iot::SensorRequest* request, 
                               iot::SensorResponse* response) override;
};