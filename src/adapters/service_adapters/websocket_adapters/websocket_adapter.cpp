/**
 * websocket_adapter.cpp -- Implementasi WebSocketAdapter
 * 
 * File ini berisi implementasi method-method WebSocketAdapter.
 * WebSocketAdapter mengkonversi data sensor (protobuf) ke JSON,
 * lalu mem-broadcast ke semua client WebSocket yang terhubung.
 */
#include "websocket_adapter.h"
#include "utils/json_helper.h"
#include <spdlog/spdlog.h>

/**
 * Constructor: simpan referensi ke WebSocket server.
 * Menggunakan initializer list untuk efisiensi.
 */
WebSocketAdapter::WebSocketAdapter(std::shared_ptr<WsServer> ws_server)
    : ws_server_(ws_server) {
}

/**
 * Inisialisasi adapter.
 * Saat ini tidak melakukan apa-apa karena WsServer
 * sudah diinisialisasi terlebih dahulu di main().
 * Method ini ada untuk memenuhi kontrak ITransportAdapter.
 */
bool WebSocketAdapter::init() {
    spdlog::info("WebSocket Adapter: Initialized");
    return true;
}

/**
 * Kirim data sensor ke semua WebSocket client.
 * 
 * Proses:
 *   1. Cek apakah WebSocket server tersedia
 *   2. Konversi SensorRequest ke format JSON menggunakan utils::sensor_to_json()
 *   3. Broadcast JSON ke semua client yang terhubung via WsServer::broadcast()
 * 
 * Format JSON yang dikirim:
 *   {"sensor_id": 1, "temperature": 25.5, "humidity": 60.0, "location": "Room A"}
 * 
 * @param request Data sensor dalam format protobuf
 */
void WebSocketAdapter::send(const iot::SensorRequest& request) {
    if (!ws_server_) {
        spdlog::warn("WebSocket Adapter: Server not available");
        return;  // Tidak bisa kirim tanpa server
    }

    // Konversi protobuf ke JSON string menggunakan RapidJSON
    std::string json_payload = utils::sensor_to_json(&request);

    // Broadcast ke semua client WebSocket yang terhubung
    ws_server_->broadcast(json_payload);
    spdlog::debug("WebSocket Adapter: Sent message");
}

/**
 * Nama adapter untuk keperluan logging dan identifikasi.
 * @return String "WebSocket"
 */
std::string WebSocketAdapter::name() const {
    return "WebSocket";
}
