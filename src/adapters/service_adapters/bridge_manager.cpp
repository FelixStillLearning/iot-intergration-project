#include "bridge_manager.h"
#include <spdlog/spdlog.h>

void BridgeManager::add_adapter(std::shared_ptr<ITransportAdapter> adapter) {
    if (!adapter) {
        spdlog::warn("Bridge: Attempted to add null adapter");
        return;
    }
    
    adapters_.push_back(adapter);
    spdlog::info("Bridge: Adapter registered - {}", adapter->name());
}

void BridgeManager::broadcast_sensor_data(const iot::SensorRequest& request) {
    spdlog::debug("Bridge: Broadcasting sensor data - ID: {}", request.sensor_id());
    
    for (auto& adapter : adapters_) {
        if (adapter) {
            adapter->send(request);
        }
    }
}
