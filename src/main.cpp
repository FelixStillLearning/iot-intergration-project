#include "utils/logger.hpp"
#include "grpc/sensor_service.hpp"
#include <grpcpp/grpcpp.h>
#include <memory>

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    SensorServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    spdlog::info("IoT Bridge System active at {}", server_address);
    server->Wait();
}

int main() {
    utils::init_logger();
    RunServer();
    return 0;
}