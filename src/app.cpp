#include "utils/log_util/logger.h"
#include "controllers/sensor_controller.h"
#include "websocket/ws_server.h"
#include "dds/dds_publisher.h"
#include "adapters/service_adapters/bridge_manager.h"
#include "adapters/service_adapters/websocket_adapters/websocket_adapter.h"
#include "adapters/service_adapters/dds_adapters/dds_adapter.h"
#include "handlers/sensor_data_log_handler/sensor_data_log_handler.h"
#include "handlers/sensor_data_validator/sensor_data_validator.h"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <thread>
#include <cstdlib>
#include <string>
#include <vector>

namespace {
std::shared_ptr<WsServer> g_ws_server;
std::shared_ptr<DdsPublisher> g_dds_pub;
std::shared_ptr<BridgeManager> g_bridge;

std::string get_env_string(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    if (value && *value) {
        return std::string(value);
    }
    return fallback;
}

int get_env_int(const char* name, int fallback) {
    const char* value = std::getenv(name);
    if (!value || !*value) {
        return fallback;
    }
    try {
        return std::stoi(value);
    } catch (...) {
        return fallback;
    }
}

void run_grpc_server() {
    std::string host = get_env_string("GRPC_HOST", "0.0.0.0");
    int port = get_env_int("GRPC_PORT", 50051);
    std::string server_address = host + ":" + std::to_string(port);
    auto service = std::make_shared<SensorController>(g_bridge);

    auto log_handler = std::make_shared<SensorDataLogHandler>();
    auto validator   = std::make_shared<SensorDataValidator>();

    service->add_observer(log_handler);  
    service->add_observer(validator);     

    spdlog::info("Observers: Registered {} handler(s) to SensorController", 2);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(service.get());

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    spdlog::info("[gRPC] Server active at {}", server_address);
    server->Wait();
}

void run_ws_server() {
    int port = get_env_int("WS_PORT", 9002);
    spdlog::info("[WebSocket] Server starting at port {}", port);
    g_ws_server->run(static_cast<uint16_t>(port));
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
    
    std::string config_path = get_env_string("DDS_CONFIG_FILE", "rtps.ini");
    std::vector<std::string> arg_strings;
    std::vector<char*> new_argv;
    for (int i = 0; i < argc; ++i) {
        arg_strings.push_back(argv[i]);
    }
    arg_strings.push_back("-DCPSConfigFile");
    arg_strings.push_back(config_path);
    for (auto& s : arg_strings) {
        new_argv.push_back(&s[0]);
    }
    new_argv.push_back(nullptr);
    int new_argc = static_cast<int>(new_argv.size() - 1);

    if (!g_dds_pub->init(new_argc, new_argv.data())) {
        spdlog::error("Failed to initialize DDS Publisher");
        return 1;
    }
    
    init_bridge();
    
    std::thread ws_thread(run_ws_server);
    ws_thread.detach();

    run_grpc_server();

    return 0;
}