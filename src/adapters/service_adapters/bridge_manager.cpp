/**
 * bridge_manager.cpp -- Implementasi BridgeManager
 * 
 * File ini berisi implementasi method-method BridgeManager.
 * BridgeManager adalah pusat distribusi data sensor ke berbagai transport.
 * 
 * Alur data melalui BridgeManager:
 *   SensorController::SendSensorData()
 *       -> bridge_->broadcast_sensor_data(request)
 *           -> WebSocketAdapter::send()  (kirim JSON ke browser)
 *           -> DdsAdapter::send()        (kirim ke DDS network)
 */
#include "bridge_manager.h"
#include <spdlog/spdlog.h>

/**
 * Daftarkan transport adapter baru.
 * 
 * Validasi: cek apakah adapter tidak null sebelum menambahkan.
 * Adapter yang null bisa terjadi jika ada error saat pembuatan object.
 * 
 * @param adapter Shared pointer ke transport adapter
 */
void BridgeManager::add_adapter(std::shared_ptr<ITransportAdapter> adapter) {
    if (!adapter) {
        spdlog::warn("Bridge: Attempted to add null adapter");
        return;  // Tolak adapter null, jangan crash
    }
    
    adapters_.push_back(adapter);  // Tambahkan ke daftar adapter
    spdlog::info("Bridge: Adapter registered - {}", adapter->name());
}

/**
 * Broadcast data sensor ke semua adapter yang terdaftar.
 * 
 * Iterasi melalui semua adapter dan panggil send() satu per satu.
 * Jika salah satu adapter gagal, adapter lainnya tetap akan dipanggil
 * (tidak ada early return saat error di satu adapter).
 * 
 * @param request Data sensor yang akan dikirim ke semua transport
 */
void BridgeManager::broadcast_sensor_data(const iot::SensorRequest& request) {
    spdlog::debug("Bridge: Broadcasting sensor data - ID: {}", request.sensor_id());
    
    // Iterasi semua adapter dan kirim data
    for (auto& adapter : adapters_) {
        if (adapter) {          // Defensive check: pastikan adapter valid
            adapter->send(request);  // Kirim data via transport adapter
        }
    }
}
