#pragma once
#include "adapters/interface_adapters/interface_transport_adapter.h"
#include "dds/dds_publisher.h"
#include <memory>

class DdsAdapter : public ITransportAdapter {
public:
    explicit DdsAdapter(std::shared_ptr<DdsPublisher> dds_publisher);
    
    bool init() override;
    void send(const iot::SensorRequest& request) override;
    std::string name() const override;

private:
    std::shared_ptr<DdsPublisher> dds_publisher_;
};
