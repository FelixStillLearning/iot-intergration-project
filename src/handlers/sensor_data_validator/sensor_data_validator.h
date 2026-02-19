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
    void on_sensor_data(const iot::SensorRequest& request) override;

    std::string observer_name() const override;

    /// Getter — berapa total anomali yang terdeteksi
    int get_anomaly_count() const;

    /// Getter — berapa total data valid yang terdeteksi
    int get_valid_count() const;

private:
    int anomaly_count_ = 0;  // Counter anomali yang terdeteksi
    int valid_count_ = 0;    // Counter data yang valid
};
