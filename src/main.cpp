#include "utils/logger.h"
#include "grpc/sensor_service.h"
#include "websocket/ws_server.h"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <thread> 

WsServer g_ws_server;

void RunGrpcServer() {
    std::string server_address("0.0.0.0:50051");
    SensorServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    spdlog::info(" [gRPC] Server active at {}", server_address);
    server->Wait();
}

void RunWsServer() {
    spdlog::info(" [WS] Server starting at port 9002");
    g_ws_server.run(9002); 
}

int main() {
    utils::init_logger();
    std::thread ws_thread(RunWsServer);
    ws_thread.detach(); 

    RunGrpcServer();

    return 0;
}