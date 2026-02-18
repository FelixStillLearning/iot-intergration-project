#include "dds_adapter.h"
#include <spdlog/spdlog.h>

DdsAdapter::DdsAdapter(std::shared_ptr<DdsPublisher> dds_publisher)
    : dds_publisher_(dds_publisher) {
}

bool DdsAdapter::init() {
    spdlog::info("DDS Adapter: Initialized");
    return true;
}

void DdsAdapter::send(const iot::SensorRequest& request) {
    if (!dds_publisher_) {
        spdlog::warn("DDS Adapter: Publisher not available");
        return;
    }

    dds_publisher_->publish(
        request.sensor_id(),
        request.sensor_name(),
        request.temperature(),
        request.humidity(),
        request.pressure(),
        request.light_intensity(),
        request.timestamp(),
        request.location()
    );
    spdlog::debug("DDS Adapter: Sent message");
}

std::string DdsAdapter::name() const {
    return "DDS";
}
