#include "sensor_data_validator.h"

void SensorDataValidator::on_sensor_data(const iot::SensorRequest& request) {
    bool is_valid = true;

    // Validasi 1: Sensor ID harus positif
    if (request.sensor_id() <= 0) {
        spdlog::warn("[Validator] INVALID: sensor_id={} (harus > 0)", 
                     request.sensor_id());
        is_valid = false;
    }

    // Validasi 2: Temperature dalam range wajar (-50 to 100°C)
    if (request.temperature() < -50.0 || request.temperature() > 100.0) {
        spdlog::warn("[Validator]  ABNORMAL TEMP: {}°C (range normal: -50 to 100)", 
                     request.temperature());
        is_valid = false;
        ++anomaly_count_;
    }

    // Validasi 3: Humidity dalam range 0-100%
    if (request.humidity() < 0.0 || request.humidity() > 100.0) {
        spdlog::warn("[Validator]  INVALID HUMIDITY: {}% (range: 0-100)", 
                     request.humidity());
        is_valid = false;
        ++anomaly_count_;
    }

    // Validasi 4: Pressure dalam range wajar (300-1100 hPa)
    if (request.pressure() < 300.0 || request.pressure() > 1100.0) {
        spdlog::warn("[Validator]  ABNORMAL PRESSURE: {} hPa (range normal: 300-1100)", 
                     request.pressure());
        is_valid = false;
        ++anomaly_count_;
    }

    // Validasi 5: Light intensity harus >= 0
    if (request.light_intensity() < 0.0) {
        spdlog::warn("[Validator]  INVALID LIGHT: {} lux (must be >= 0)", 
                     request.light_intensity());
        is_valid = false;
        ++anomaly_count_;
    }

    // Summary
    if (is_valid) {
        spdlog::info("[Validator]  Data VALID");
        ++valid_count_;
    }
}

std::string SensorDataValidator::observer_name() const {
    return "SensorDataValidator";
}

int SensorDataValidator::get_anomaly_count() const {
    return anomaly_count_;
}

int SensorDataValidator::get_valid_count() const {
    return valid_count_;
}
