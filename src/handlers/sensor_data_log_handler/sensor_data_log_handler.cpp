#include "sensor_data_log_handler.h"

void SensorDataLogHandler::on_sensor_data(const iot::SensorRequest& request) {
    spdlog::info("┌─────────── [DataLogger] Sensor Report ───────────┐");
    spdlog::info("│ Sensor ID      : {}",   request.sensor_id());
    spdlog::info("│ Sensor Name    : {}",   request.sensor_name());
    spdlog::info("│ Temperature    : {}°C", request.temperature());
    spdlog::info("│ Humidity       : {}%",  request.humidity());
    spdlog::info("│ Pressure       : {} hPa", request.pressure());
    spdlog::info("│ Light Intensity: {} lux",  request.light_intensity());
    spdlog::info("│ Timestamp      : {}",   request.timestamp());
    spdlog::info("│ Location       : {}",   request.location());
    spdlog::info("└─────────────────────────────────────────────────┘");

    ++log_count_;
    spdlog::debug("[DataLogger] Total logged: {} messages", log_count_);
}

std::string SensorDataLogHandler::observer_name() const {
    return "SensorDataLogHandler";
}

int SensorDataLogHandler::get_log_count() const {
    return log_count_;
}
