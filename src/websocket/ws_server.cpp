/**
 * ws_server.cpp -- Implementasi WebSocket Server
 * 
 * File ini berisi implementasi WsServer yang menggunakan library websocketpp.
 * Server ini berjalan di thread terpisah dan mengelola koneksi WebSocket
 * untuk streaming data sensor real-time ke client.
 * 
 * Library yang digunakan:
 *   - websocketpp : library WebSocket C++ yang ringan dan portable
 *   - ASIO        : library I/O asynchronous (backend network websocketpp)
 */
#include "ws_server.h"
#include <websocketpp/common/connection_hdl.hpp>
#include <functional>
#include <spdlog/spdlog.h>

/**
 * Constructor WsServer.
 * 
 * Setup konfigurasi awal:
 *   1. Matikan semua logging internal websocketpp (terlalu verbose)
 *   2. Inisialisasi ASIO (backend network I/O)
 *   3. Aktifkan SO_REUSEADDR agar port bisa langsung dipakai setelah restart
 *   4. Set callback handler untuk event open (connect) dan close (disconnect)
 */
WsServer::WsServer() {
    // Matikan logging internal websocketpp -- kita pakai spdlog sendiri
    m_server.clear_access_channels(websocketpp::log::alevel::all);
    m_server.clear_error_channels(websocketpp::log::elevel::all);

    m_server.init_asio();          // Inisialisasi ASIO backend
    m_server.set_reuse_addr(true); // SO_REUSEADDR: port langsung available setelah restart

    // Bind callback functions ke event WebSocket
    // std::bind menghubungkan method class ke handler websocketpp
    m_server.set_open_handler(std::bind(&WsServer::on_open, this, std::placeholders::_1));
    m_server.set_close_handler(std::bind(&WsServer::on_close, this, std::placeholders::_1));
}

/**
 * Jalankan WebSocket server.
 * 
 * Urutan:
 *   1. listen() -- bind ke port dan mulai mendengarkan
 *   2. start_accept() -- mulai menerima koneksi baru
 *   3. run() -- masuk ke event loop (BLOCKING)
 * 
 * Fungsi ini BLOCKING dan harus dijalankan di thread terpisah.
 * Error handling: jika port sudah dipakai atau error lain, log dan return.
 * 
 * @param port Port number untuk listening
 */
void WsServer::run(uint16_t port) {
    try {
        m_server.listen(port);         // Bind ke port
        m_server.start_accept();       // Mulai terima koneksi baru
        spdlog::info("[WebSocket] Listening on port {}", port);
        m_server.run();                // Event loop (BLOCKING)
    } catch (const websocketpp::exception& e) {
        spdlog::error("[WebSocket] Failed to start: {}", e.what());
    } catch (const std::exception& e) {
        spdlog::error("[WebSocket] Error: {}", e.what());
    }
}

/**
 * Broadcast pesan ke semua client WebSocket yang terhubung.
 * 
 * Thread-safe: menggunakan lock_guard untuk melindungi m_connections.
 * Dipanggil dari thread gRPC (via WebSocketAdapter::send()),
 * sementara on_open/on_close dipanggil dari thread WebSocket.
 * 
 * Jika ada error saat mengirim ke satu client, error diabaikan
 * dan broadcast tetap dilanjutkan ke client lain.
 * 
 * @param message String yang akan dikirim (JSON data sensor)
 */
void WsServer::broadcast(const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);  // Lock selama iterasi
    for (const auto& hdl : m_connections) {
        websocketpp::lib::error_code ec;
        // Kirim sebagai text frame (bukan binary) karena isinya JSON
        m_server.send(hdl, message, websocketpp::frame::opcode::text, ec);
    }
}

/**
 * Callback saat client baru terhubung.
 * Dipanggil otomatis oleh websocketpp saat koneksi WebSocket berhasil.
 * Menambahkan handle koneksi ke set agar bisa di-broadcast nanti.
 * 
 * @param hdl Handle koneksi client baru
 */
void WsServer::on_open(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(m_mutex);  // Thread-safe insert
    m_connections.insert(hdl);  // Tambah ke daftar koneksi aktif
}

/**
 * Callback saat client terputus.
 * Dipanggil otomatis oleh websocketpp saat koneksi ditutup.
 * Menghapus handle koneksi dari set agar tidak di-broadcast lagi.
 * 
 * @param hdl Handle koneksi client yang terputus
 */
void WsServer::on_close(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(m_mutex);  // Thread-safe erase
    m_connections.erase(hdl);  // Hapus dari daftar koneksi aktif
}
