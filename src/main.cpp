#include <iostream> 
#include <memory>
#include <string>

#include "sensor.grpc.pb.h"
#include <grpcpp/grpcpp.h>

using std::string;
using std::unique_ptr;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using iot::SensorService;
using iot::SensorRequest;
using iot::SensorResponse;

class SensorServiceImpl final : public SensorService::Service {
    Status SendSensorData (ServerContext* context, const SensorRequest* request, SensorResponse* response) override {
        spdlog::info("=== Incoming Data Received ===");
        spdlog::info("Sensor ID   : {}", request->sensor_id());
        spdlog::info("Temperature : {} C", request->temperature());
        spdlog::info("Humidity    : {} %", request->humidity());
        spdlog::info("Location    : {}", request->location());
        
        response->set_success(true);
        response->set_message("Data recieved and connected");

        return Status::OK;
    }
};

void RunServer() {
    string server_address("0.0.0.0:50051");
    SensorServiceImpl service:

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    unique_ptr<Server> server(builder.BuildAndStart());
    cout << "Gateway Bridge is running on : " << server_address << endl;

    server->Wait();
}
 
int main() {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    
    RunServer();
    return 0;
}