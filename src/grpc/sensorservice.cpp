#include "sensor_service.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread> // Wajib untuk std::this_thread::sleep_for

// 1. UNARY RPC: Satu Request, Satu Response
grpc::Status SensorServiceImpl::SendSensorData(grpc::ServerContext* context, 
                                             const iot::SensorRequest* request, 
                                             iot::SensorResponse* response) {
    spdlog::info("Incoming gRPC -> Sensor ID: {}, Temp: {}C", 
                 request->sensor_id(), request->temperature());
    
    response->set_success(true);
    response->set_message("Bridge: Data processed successfully");
    return grpc::Status::OK;
}

// 2. CLIENT STREAMING: Banyak Request, Satu Response
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

// 3. SERVER STREAMING: Satu Request, Banyak Response (Monitor)
grpc::Status SensorServiceImpl::MonitorSensor(
    grpc::ServerContext* context,
    const iot::SensorRequest* request,
    grpc::ServerWriter<iot::SensorResponse>* writer) {
    
    spdlog::info("[Monitor] Starting stream for Sensor ID: {}", request->sensor_id());

    // Simulasi pengiriman data kontinu sebanyak 5 kali
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

        writer->Write(res); // Mengirim pesan ke client stream
        spdlog::info("[Monitor] Sent update #{} to client", i);

        // Jeda 1 detik antar pengiriman
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return grpc::Status::OK;
}

// 4. BIDIRECTIONAL STREAMING: Banyak Request, Banyak Response (Interactive)
grpc::Status SensorServiceImpl::InteractiveSensor(
    grpc::ServerContext* context,
    grpc::ServerReaderWriter<iot::SensorResponse, iot::SensorRequest>* stream) {
    
    iot::SensorRequest request;
    spdlog::info("[Interactive] Session started");

    // Server membaca request dan langsung membalas
    while (stream->Read(&request)) {
        spdlog::info("[Interactive] Received Sensor ID: {} from {}", request.sensor_id(), request.location());

        iot::SensorResponse response;
        response.set_success(true);
        response.set_message("Server Echo: Received data from " + request.location());
        response.set_processed_timestamp(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());

        stream->Write(response); // Langsung kirim balik balesan
    }

    spdlog::info("[Interactive] Session closed by client");
    return grpc::Status::OK;
}