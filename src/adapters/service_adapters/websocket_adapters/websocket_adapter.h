#pragma once
#include "adapters/interface_adapters/interface_transport_adapter.h"
#include "websocket/ws_server.h"
#include <memory>

/**
 * WebSocketAdapter -- Concrete Transport Adapter untuk WebSocket
 * 
 * Adapter ini menjembatani SensorController ke WebSocket server.
 * Ketika data sensor masuk via gRPC, BridgeManager memanggil send()
 * pada WebSocketAdapter, yang kemudian:
 *   1. Mengkonversi SensorRequest (protobuf) ke format JSON
 *   2. Mem-broadcast JSON tersebut ke semua client WebSocket yang terhubung
 * 
 * Alur data:
 *   BridgeManager::broadcast_sensor_data()
 *       -> WebSocketAdapter::send(request)
 *           -> utils::sensor_to_json(request)  (konversi ke JSON)
 *           -> WsServer::broadcast(json)       (kirim ke semua client)
 * 
 * WebSocket digunakan untuk:
 *   - Streaming data real-time ke browser/dashboard
 *   - Koneksi persistent (tidak perlu polling seperti HTTP)
 *   - Komunikasi bidirectional antara server dan client
 * 
 * Class ini TIDAK memiliki logic WebSocket secara langsung.
 * Semua logic WebSocket ada di WsServer. WebSocketAdapter hanya berperan
 * sebagai "adapter" yang menerjemahkan SensorRequest ke format JSON.
 */
class WebSocketAdapter : public ITransportAdapter {
public:
    /**
     * Constructor: terima shared pointer ke WebSocket server.
     * WsServer sudah dibuat di main() sebelum adapter dibuat.
     * @param ws_server Instance WebSocket server yang aktif
     */
    explicit WebSocketAdapter(std::shared_ptr<WsServer> ws_server);
    
    /**
     * Inisialisasi adapter. Saat ini hanya log bahwa adapter siap.
     * @return true selalu (WsServer sudah diinit di main())
     */
    bool init() override;

    /**
     * Kirim data sensor ke semua WebSocket client.
     * Mengkonversi SensorRequest ke JSON lalu broadcast.
     * @param request Data sensor yang akan dikirim via WebSocket
     */
    void send(const iot::SensorRequest& request) override;

    /**
     * Nama adapter untuk logging.
     * @return "WebSocket"
     */
    std::string name() const override;

private:
    std::shared_ptr<WsServer> ws_server_;  // Referensi ke WebSocket server
};
