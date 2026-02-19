/**
 * interface_bridge_manager.h -- Definisi interface IBridgeManager
 *
 * File ini mendefinisikan kontrak (pure virtual interface) untuk Bridge Manager.
 * Semua class yang ingin menjadi "pengelola transport adapter" harus
 * mengimplementasikan interface ini. Ini adalah bagian dari Bridge Pattern.
 */
#pragma once
#include <memory>
#include "sensor.pb.h"

// Forward declaration -- hanya butuh nama class untuk parameter shared_ptr
class ITransportAdapter;

/**
 * IBridgeManager Interface (Pure Virtual)
 * 
 * Kontrak untuk class yang bertugas mengelola transport adapters.
 * Bridge Manager bertanggung jawab:
 *   - add_adapter()           -> daftarkan transport adapter baru (WebSocket, DDS, dll)
 *   - broadcast_sensor_data() -> kirim data sensor ke SEMUA adapter yang terdaftar
 * 
 * Konsep Bridge Pattern:
 *   Memisahkan "abstraksi" (apa yang mau dilakukan: broadcast data)
 *   dari "implementasi" (bagaimana caranya: via WebSocket, DDS, MQTT, dll).
 * 
 *   Dengan ini, menambah transport baru (misalnya MQTT) cukup:
 *   1. Buat class MqttAdapter yang implement ITransportAdapter
 *   2. Daftarkan ke BridgeManager via add_adapter()
 *   3. Tidak perlu ubah code di SensorController atau BridgeManager
 * 
 * Dalam project ini:
 *   IBridgeManager (interface)
 *       -> BridgeManager (concrete implementation)
 */
class IBridgeManager {
public:
    virtual ~IBridgeManager() = default;
    
    /**
     * Daftarkan transport adapter baru ke bridge.
     * Setelah didaftarkan, adapter akan menerima data setiap kali broadcast dipanggil.
     * @param adapter Shared pointer ke adapter yang implement ITransportAdapter
     */
    virtual void add_adapter(std::shared_ptr<ITransportAdapter> adapter) = 0;

    /**
     * Broadcast data sensor ke semua adapter yang terdaftar.
     * Setiap adapter akan memanggil send() masing-masing.
     * @param request Data sensor dari gRPC yang akan dikirim
     */
    virtual void broadcast_sensor_data(const iot::SensorRequest& request) = 0;
};
