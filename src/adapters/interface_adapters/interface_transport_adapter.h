#pragma once
#include "sensor.pb.h"

class ITransportAdapter {
public:
    virtual ~ITransportAdapter() = default;
    
    virtual bool init() = 0;
    virtual void send(const iot::SensorRequest& request) = 0;
    virtual std::string name() const = 0;
};
