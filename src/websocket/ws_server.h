#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <set>
#include <mutex>

/**
 * Type alias untuk websocketpp server tanpa TLS (tanpa enkripsi).
 * Menggunakan konfigurasi asio_no_tls karena ini komunikasi internal.
 * Untuk production dengan koneksi dari internet, pertimbangkan asio_tls.
 */
typedef websocketpp::server<websocketpp::config::asio> server;

/**
 * WsServer -- WebSocket Server menggunakan websocketpp
 * 
 * Class ini mengelola WebSocket server yang bertugas:
 *   - Menerima koneksi dari client (browser, dashboard, monitoring tool)
 *   - Menyimpan daftar koneksi yang aktif
 *   - Mem-broadcast pesan (JSON data sensor) ke SEMUA client yang terhubung
 * 
 * WebSocket dipilih karena:
 *   - Koneksi persistent (tidak perlu buka-tutup seperti HTTP)
 *   - Server bisa push data ke client (tidak hanya client yang request)
 *   - Cocok untuk streaming data real-time seperti data sensor
 * 
 * Thread safety:
 *   - m_mutex melindungi m_connections dari concurrent access
 *   - broadcast() bisa dipanggil dari thread gRPC
 *   - on_open/on_close dipanggil dari thread WebSocket
 * 
 * Alur data:
 *   WebSocketAdapter::send()
 *       -> WsServer::broadcast(json)
 *           -> kirim ke semua connection handle di m_connections
 */
class WsServer {
public:
    /**
     * Constructor: setup WebSocket server.
     * Konfigurasi: matikan logging internal websocketpp, init ASIO, set handler.
     */
    WsServer();

    /**
     * Jalankan server di port tertentu.
     * BLOCKING -- fungsi ini tidak return sampai server di-shutdown.
     * Harus dijalankan di thread terpisah (lihat run_ws_server() di app.cpp).
     * @param port Port number untuk listening (default: 9002)
     */
    void run(uint16_t port);

    /**
     * Broadcast pesan ke SEMUA client yang terhubung.
     * Thread-safe: menggunakan mutex untuk melindungi daftar koneksi.
     * @param message String yang akan dikirim (biasanya JSON data sensor)
     */
    void broadcast(const std::string& message);

private:
    /**
     * Callback: dipanggil saat client baru terhubung.
     * Menambahkan connection handle ke daftar.
     * @param hdl Handle koneksi client baru
     */
    void on_open(websocketpp::connection_hdl hdl);

    /**
     * Callback: dipanggil saat client terputus.
     * Menghapus connection handle dari daftar.
     * @param hdl Handle koneksi client yang terputus
     */
    void on_close(websocketpp::connection_hdl hdl);

    server m_server;  // Instance websocketpp server

    /**
     * Daftar semua koneksi WebSocket yang aktif.
     * Menggunakan std::set dengan owner_less comparator karena
     * connection_hdl adalah weak_ptr yang tidak bisa di-compare secara default.
     */
    std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> m_connections;

    std::mutex m_mutex;  // Mutex untuk thread-safety akses ke m_connections
};
