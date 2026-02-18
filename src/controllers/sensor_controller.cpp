#include "sensor_controller.h"
#include "adapters/service_adapters/bridge_manager.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>

SensorController::SensorController(std::shared_ptr<BridgeManager> bridge)
    : bridge_(bridge) {
}

grpc::Status SensorController::SendSensorData(grpc::ServerContext* context, 
                                             const iot::SensorRequest* request, 
                                             iot::SensorResponse* response) {
    spdlog::info("Incoming gRPC -> Sensor ID: {}, Temp: {}C", 
                 request->sensor_id(), request->temperature());
    
    if (bridge_) {
        bridge_->broadcast_sensor_data(*request);
    }
    
    response->set_success(true);
    response->set_message("Bridge: Data processed successfully");
    return grpc::Status::OK;
}

grpc::Status SensorController::StreamSensorData(
    grpc::ServerContext* context,
    grpc::ServerReader<iot::SensorRequest>* reader,
    iot::SensorResponse* response) {
    (void)context;

    iot::SensorRequest request;
    int count = 0;
    while (reader->Read(&request)) {
        spdlog::info("[Stream] Sensor ID: {}, Temp: {}C", request.sensor_id(), request.temperature());
        if (bridge_) {
            bridge_->broadcast_sensor_data(request);
        }
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

grpc::Status SensorController::MonitorSensor(
    grpc::ServerContext* context,
    const iot::SensorRequest* request,
    grpc::ServerWriter<iot::SensorResponse>* writer) {
    
    spdlog::info("[Monitor] Starting stream for Sensor ID: {}", request->sensor_id());

    for (int i = 1; i <= 5; ++i) {
        if (context->IsCancelled()) {
            return grpc::Status::CANCELLED;
        }

        iot::SensorResponse res;
        res.set_success(true);
        res.set_message("Real-time update #" + std::to_string(i));
        res.set_processed_timestamp(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());

        writer->Write(res);
        spdlog::info("[Monitor] Sent update #{} to client", i);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return grpc::Status::OK;
}

grpc::Status SensorController::InteractiveSensor(
    grpc::ServerContext* context,
    grpc::ServerReaderWriter<iot::SensorResponse, iot::SensorRequest>* stream) {
    
    iot::SensorRequest request;
    spdlog::info("[Interactive] Session started");

    while (stream->Read(&request)) {
        spdlog::info("[Interactive] Received Sensor ID: {} from {}", request.sensor_id(), request.location());
        if (bridge_) {
            bridge_->broadcast_sensor_data(request);
        }

        iot::SensorResponse response;
        response.set_success(true);
        response.set_message("Server Echo: Received data from " + request.location());
        response.set_processed_timestamp(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());

        stream->Write(response);
    }

    spdlog::info("[Interactive] Session closed by client");
    return grpc::Status::OK;
}
