#include "websocket_adapter.h"
#include "utils/json_helper.h"
#include <spdlog/spdlog.h>

WebSocketAdapter::WebSocketAdapter(std::shared_ptr<WsServer> ws_server)
    : ws_server_(ws_server) {
}

bool WebSocketAdapter::init() {
    spdlog::info("WebSocket Adapter: Initialized");
    return true;
}

void WebSocketAdapter::send(const iot::SensorRequest& request) {
    if (!ws_server_) {
        spdlog::warn("WebSocket Adapter: Server not available");
        return;
    }

    std::string json_payload = utils::sensor_to_json(&request);
    ws_server_->broadcast(json_payload);
    spdlog::debug("WebSocket Adapter: Sent message");
}

std::string WebSocketAdapter::name() const {
    return "WebSocket";
}
