#include "utils/log_util/logger.h"
#include "controllers/sensor_controller.h"
#include "websocket/ws_server.h"
#include "dds/dds_publisher.h"
#include "adapters/service_adapters/bridge_manager.h"
#include "adapters/service_adapters/websocket_adapters/websocket_adapter.h"
#include "adapters/service_adapters/dds_adapters/dds_adapter.h"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <thread>

namespace {
std::shared_ptr<WsServer> g_ws_server;
std::shared_ptr<DdsPublisher> g_dds_pub;
std::shared_ptr<BridgeManager> g_bridge;

void run_grpc_server() {
    std::string server_address("0.0.0.0:50051");
    auto service = std::make_shared<SensorController>(g_bridge);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(service.get());

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    spdlog::info("[gRPC] Server active at {}", server_address);
    server->Wait();
}

void run_ws_server() {
    spdlog::info("[WebSocket] Server starting at port 9002");
    g_ws_server->run(9002);
}

void init_bridge() {
    g_bridge = std::make_shared<BridgeManager>();
    
    auto ws_adapter = std::make_shared<WebSocketAdapter>(g_ws_server);
    auto dds_adapter = std::make_shared<DdsAdapter>(g_dds_pub);
    
    g_bridge->add_adapter(ws_adapter);
    g_bridge->add_adapter(dds_adapter);
    
    spdlog::info("Bridge: Initialized with {} adapters", 2);
}
}

int main(int argc, char* argv[]) {
    utils::init_logger();
    
    g_ws_server = std::make_shared<WsServer>();
    g_dds_pub = std::make_shared<DdsPublisher>();
    
    if (!g_dds_pub->init(argc, argv)) {
        spdlog::error("Failed to initialize DDS Publisher");
        return 1;
    }
    
    init_bridge();
    
    std::thread ws_thread(run_ws_server);
    ws_thread.detach();

    run_grpc_server();

    return 0;
}