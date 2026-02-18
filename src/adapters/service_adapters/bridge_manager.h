#pragma once
#include "adapters/interface_adapters/interface_transport_adapter.h"
#include <vector>
#include <memory>
#include "sensor.grpc.pb.h"

class BridgeManager {
public:
    BridgeManager() = default;
    
    void add_adapter(std::shared_ptr<ITransportAdapter> adapter);
    void broadcast_sensor_data(const iot::SensorRequest& request);
    
private:
    std::vector<std::shared_ptr<ITransportAdapter>> adapters_;
};
