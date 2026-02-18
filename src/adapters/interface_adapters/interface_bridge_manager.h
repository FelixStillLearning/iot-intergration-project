#pragma once
#include <memory>
#include "sensor.pb.h"

class ITransportAdapter;

class IBridgeManager {
public:
    virtual ~IBridgeManager() = default;
    
    virtual void add_adapter(std::shared_ptr<ITransportAdapter> adapter) = 0;
    virtual void broadcast_sensor_data(const iot::SensorRequest& request) = 0;
};
