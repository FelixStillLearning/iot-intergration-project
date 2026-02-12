#include <iostream> 
#include <memory>
#include <String>

#include "sensor.grpc.pb.h"
#include <grpcpp/grpcpp.h>

using std::cout;
using std::endl;
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
        cout << "Incoming Data :" << endl;
        cout << "Sensor ID   : " <<request->sensor_id() << endl;
        cout << "Temperature : " <<request->temperature() << endl;
        cout << "Humidity    : " <<request->humidity() << endl;
        cout << "Location    : " <<request->location() << endl;
        
        response->set_succes(true);
        response->set_message("Data recieved and connected");

        return Status::OK;
    }
};

void RunServer() {
    string server_address("0.0.0.0:50051");
    SensorServiceImpl service:

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.Registerservice(&service);

    unique_ptr<Server> server(builder.BuildAndStart());
    cout << "Gateway Bridge is running on : " << server_address << endl;

    server->Wait();
}
 
int main() {
    RunServer();
    return 0;
}