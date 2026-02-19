#pragma once
#include "sensor.pb.h"

/**
 * ITransportAdapter Interface (Pure Virtual)
 * 
 * Kontrak untuk semua transport adapter dalam sistem.
 * Setiap adapter mewakili SATU cara pengiriman data sensor.
 * 
 * Method yang harus diimplementasi:
 *   - init() -> inisialisasi koneksi/resource adapter
 *   - send() -> kirim satu data sensor melalui transport ini
 *   - name() -> nama adapter (untuk logging dan identifikasi)
 * 
 * Concrete implementations dalam project ini:
 *   ITransportAdapter (interface)
 *       -> WebSocketAdapter : kirim data via WebSocket ke browser/frontend
 *       -> DdsAdapter       : kirim data via OpenDDS ke node DDS lain
 * 
 * Untuk menambah transport baru (misalnya MQTT, Kafka, HTTP):
 *   1. Buat class baru yang inherit ITransportAdapter
 *   2. Implement init(), send(), dan name()
 *   3. Daftarkan ke BridgeManager di init_bridge() pada app.cpp
 */
class ITransportAdapter {
public:
    virtual ~ITransportAdapter() = default;
    
    /**
     * Inisialisasi adapter (setup koneksi, alokasi resource, dll).
     * @return true jika berhasil, false jika gagal
     */
    virtual bool init() = 0;

    /**
     * Kirim data sensor melalui transport ini.
     * Dipanggil oleh BridgeManager saat broadcast_sensor_data().
     * @param request Data sensor yang akan dikirim
     */
    virtual void send(const iot::SensorRequest& request) = 0;

    /**
     * Nama adapter untuk keperluan logging dan identifikasi.
     * Contoh: "WebSocket", "DDS"
     * @return Nama string adapter
     */
    virtual std::string name() const = 0;
};
