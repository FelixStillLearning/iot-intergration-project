#include "sensor_service.h"
#include <spdlog/spdlog.h>
#include <chrono>

grpc::Status SensorServiceImpl::SendSensorData(grpc::ServerContext* context, 
                                             const iot::SensorRequest* request, 
                                             iot::SensorResponse* response) {
    spdlog::info("Incoming gRPC -> Sensor ID: {}, Temp: {}C", 
                 request->sensor_id(), request->temperature());
    
    
    response->set_success(true);
    response->set_message("Bridge: Data processed successfully");
    return grpc::Status::OK;
}

grpc::Status SensorServiceImpl::StreamSensorData(
    grpc::ServerContext* context,
    grpc::ServerReader<iot::SensorRequest>* reader,
    iot::SensorResponse* response) {
    (void)context;

    iot::SensorRequest request;
    int count = 0;
    while (reader->Read(&request)) {
        spdlog::info("[Stream] Sensor ID: {}, Temp: {}C", request.sensor_id(), request.temperature());
        ++count;
    }

    response->set_success(true);
    response->set_message("Bridge: Stream processed");
    response->set_processed_timestamp(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());

    spdlog::info("[Stream] Total messages received: {}", count);
    return grpc::Status::OK;
}