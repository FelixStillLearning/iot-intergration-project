#include "sensor_service.hpp"
#include <spdlog/spdlog.h>

grpc::Status SensorServiceImpl::SendSensorData(grpc::ServerContext* context, 
                                             const iot::SensorRequest* request, 
                                             iot::SensorResponse* response) {
    // Di sinilah logika bridge bekerja
    spdlog::info("Incoming gRPC -> Sensor ID: {}, Temp: {}C", 
                 request->sensor_id(), request->temperature());
    
    // Nanti panggil dds_publisher->publish() di sini
    
    response->set_success(true);
    response->set_message("Bridge: Data processed successfully");
    return grpc::Status::OK;
}