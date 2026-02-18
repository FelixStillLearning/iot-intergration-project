#pragma once
#include "sensor.pb.h"
#include <string>

/**
 * Observer Interface (Pure Virtual / Abstract)
 * 
 * Setiap class yang ingin "mengamati" data sensor harus implement interface ini.
 * Ketika SensorController menerima data baru, semua Observer yang terdaftar
 * akan di-notify melalui method on_sensor_data().
 * 
 * Ini adalah bagian dari Observer Pattern:
 *   Observable (Subject) → notify → Observer 1, Observer 2, Observer 3, ...
 * 
 * Contoh concrete observers:
 *   - SensorDataLogHandler  → log detail data ke console
 *   - SensorDataValidator   → validasi data, alert jika abnormal
 */
class Observer {
public:
    virtual ~Observer() = default;

    /**
     * Dipanggil oleh Observable saat ada data sensor baru.
     * @param request Data sensor yang diterima dari gRPC client
     */
    virtual void on_sensor_data(const iot::SensorRequest& request) = 0;

    /**
     * Nama observer (untuk logging/debug)
     */
    virtual std::string observer_name() const = 0;
};