#pragma once
#include "handlers/observer/observer.h"
#include <spdlog/spdlog.h>

/**
 * SensorDataLogHandler — Concrete Observer #1
 * 
 * Observer ini bertugas mencatat (log) SEMUA data sensor yang masuk
 * secara detail ke console. Ini berguna untuk:
 *   - Debugging (lihat semua field data sensor)
 *   - Audit trail (record setiap data yang melewati sistem)
 *   - Monitoring (lihat real-time apa yang terjadi)
 * 
 * Observer Pattern chain:
 *   SensorController (Observable)
 *       → notify_observers(request)
 *           → SensorDataLogHandler::on_sensor_data(request)  ← INI
 *           → SensorDataValidator::on_sensor_data(request)
 * 
 * Tanpa Observer Pattern, nantinya harus menulis log ini LANGSUNG
 * di SensorController — itu membuat controller "gemuk" dan susah di-maintain.
 * Dengan Observer, logic logging terpisah di class sendiri.
 */
class SensorDataLogHandler : public Observer {
public:
    /**
     * Dipanggil otomatis oleh Observable saat ada data sensor baru.
     * Log semua field sensor ke console dengan format terstruktur.
     */
    void on_sensor_data(const iot::SensorRequest& request) override;

    std::string observer_name() const override;

    /// Getter — berapa total data yang sudah di-log
    int get_log_count() const;

private:
    int log_count_ = 0;  // Counter untuk tracking jumlah data yang di-log
};
