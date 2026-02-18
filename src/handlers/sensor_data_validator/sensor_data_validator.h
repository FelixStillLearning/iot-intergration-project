#pragma once
#include "handlers/observer/observer.h"
#include <spdlog/spdlog.h>

/**
 * SensorDataValidator — Concrete Observer #2
 * 
 * Observer ini bertugas MEMVALIDASI data sensor yang masuk.
 * Jika data di luar batas normal, akan mengeluarkan WARNING.
 * 
 * Contoh real-world use case:
 *   - Suhu > 50°C → mungkin sensor rusak, atau kebakaran
 *   - Humidity < 0 atau > 100 → data korup/invalid
 *   - Sensor ID negatif → error di client
 * 
 * PENTING: Observer ini TIDAK menghentikan alur data.
 * Data tetap diteruskan ke Bridge (WebSocket + DDS).
 * Observer hanya "mengamati" dan bereaksi — tidak mengubah flow.
 * 
 * Ini mendemonstrasikan prinsip:
 *   - Single Responsibility: Validator HANYA validasi, tidak kirim data
 *   - Open/Closed Principle: Tambah aturan validasi tanpa ubah controller
 *   - Separation of Concerns: Logic validasi terpisah dari logic transport
 */
class SensorDataValidator : public Observer {
public:
    void on_sensor_data(const iot::SensorRequest& request) override {
        bool is_valid = true;

        // Validasi 1: Sensor ID harus positif
        if (request.sensor_id() <= 0) {
            spdlog::warn("[Validator] ⚠ INVALID: sensor_id={} (harus > 0)", 
                         request.sensor_id());
            is_valid = false;
        }

        // Validasi 2: Temperature dalam range wajar (-50 to 100°C)
        if (request.temperature() < -50.0 || request.temperature() > 100.0) {
            spdlog::warn("[Validator] ⚠ ABNORMAL TEMP: {}°C (range normal: -50 to 100)", 
                         request.temperature());
            is_valid = false;
            ++anomaly_count_;
        }

        // Validasi 3: Humidity dalam range 0-100%
        if (request.humidity() < 0.0 || request.humidity() > 100.0) {
            spdlog::warn("[Validator] ⚠ INVALID HUMIDITY: {}% (range: 0-100)", 
                         request.humidity());
            is_valid = false;
            ++anomaly_count_;
        }

        // Validasi 4: Pressure dalam range wajar (300-1100 hPa)
        if (request.pressure() < 300.0 || request.pressure() > 1100.0) {
            spdlog::warn("[Validator] ⚠ ABNORMAL PRESSURE: {} hPa (range normal: 300-1100)", 
                         request.pressure());
            is_valid = false;
            ++anomaly_count_;
        }

        // Validasi 5: Sensor name tidak boleh kosong
        if (request.sensor_name().empty()) {
            spdlog::warn("[Validator] ⚠ MISSING: sensor_name is empty");
            is_valid = false;
        }

        // Validasi 6: Location tidak boleh kosong
        if (request.location().empty()) {
            spdlog::warn("[Validator] ⚠ MISSING: location is empty");
            is_valid = false;
        }

        if (is_valid) {
            spdlog::debug("[Validator] ✓ Data valid — Sensor ID: {}, Temp: {}°C", 
                          request.sensor_id(), request.temperature());
        } else {
            spdlog::warn("[Validator] ✗ Data has issues — Sensor ID: {} (total anomalies: {})", 
                         request.sensor_id(), anomaly_count_);
        }
    }

    std::string observer_name() const override {
        return "SensorDataValidator";
    }

    /// Getter — berapa total anomali yang terdeteksi
    int get_anomaly_count() const { return anomaly_count_; }

private:
    int anomaly_count_ = 0;  // Counter anomali yang terdeteksi
};
