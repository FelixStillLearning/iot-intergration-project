#pragma once
#include "sensor.grpc.pb.h"
#include <grpcpp/grpcpp.h>

class SensorServiceImpl final : public iot::SensorService::Service {
public:
    grpc::Status SendSensorData(grpc::ServerContext* context, 
                               const iot::SensorRequest* request, 
                               iot::SensorResponse* response) override;
    grpc::Status StreamSensorData(grpc::ServerContext* context,
                                  grpc::ServerReader<iot::SensorRequest>* reader,
                                  iot::SensorResponse* response) override;
};