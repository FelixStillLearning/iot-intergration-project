#pragma once
#include "adapters/interface_adapters/interface_transport_adapter.h"
#include <vector>
#include <memory>
#include "sensor.grpc.pb.h"

/**
 * BridgeManager -- Concrete Implementation dari IBridgeManager
 * 
 * Class ini mengelola kumpulan transport adapters dan bertanggung jawab
 * mendistribusikan data sensor ke semua adapter yang terdaftar.
 * 
 * Cara kerjanya sederhana:
 *   1. Adapter didaftarkan via add_adapter() saat startup (di init_bridge())
 *   2. Ketika data sensor masuk, SensorController memanggil broadcast_sensor_data()
 *   3. BridgeManager meng-iterasi semua adapter dan memanggil send() masing-masing
 * 
 * Adapter yang terdaftar saat ini:
 *   - WebSocketAdapter : kirim JSON ke browser via WebSocket
 *   - DdsAdapter       : kirim IDL message ke DDS network via OpenDDS
 * 
 * Keuntungan pattern ini:
 *   - SensorController tidak perlu tahu detail transport (loose coupling)
 *   - Menambah transport baru tidak perlu mengubah class ini
 *   - Setiap adapter bisa di-enable/disable secara independen
 */
class BridgeManager {
public:
    BridgeManager() = default;
    
    /**
     * Daftarkan adapter baru ke dalam daftar transport.
     * Adapter yang didaftarkan akan menerima data saat broadcast dipanggil.
     * @param adapter Shared pointer ke adapter (WebSocketAdapter, DdsAdapter, dll)
     */
    void add_adapter(std::shared_ptr<ITransportAdapter> adapter);

    /**
     * Kirim data sensor ke SEMUA adapter yang terdaftar.
     * Iterasi satu per satu dan panggil send() pada masing-masing.
     * @param request Data sensor dari gRPC yang akan di-broadcast
     */
    void broadcast_sensor_data(const iot::SensorRequest& request);
    
private:
    /**
     * Daftar semua transport adapter yang terdaftar.
     * Menggunakan vector karena urutan pendaftaran mungkin penting,
     * dan jumlah adapter biasanya kecil (2-5 adapter).
     */
    std::vector<std::shared_ptr<ITransportAdapter>> adapters_;
};
