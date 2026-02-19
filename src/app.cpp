/**
 * app.cpp -- Entry Point Aplikasi IoT Bridge
 * 
 * File ini adalah titik masuk utama (main) dari seluruh sistem.
 * Fungsinya:
 *   1. Inisialisasi logger untuk monitoring/debug
 *   2. Inisialisasi DDS Publisher (OpenDDS) untuk komunikasi antar-node
 *   3. Inisialisasi WebSocket Server untuk streaming data ke browser/client
 *   4. Membentuk BridgeManager yang menggabungkan DDS + WebSocket sebagai transport
 *   5. Menjalankan gRPC Server yang menerima data sensor dari client
 *   6. Mendaftarkan Observer (LogHandler, Validator) ke SensorController
 * 
 * Alur startup:
 *   main()
 *     -> init_logger()            (setup logging)
 *     -> DdsPublisher::init()     (koneksi ke DDS network)
 *     -> init_bridge()            (daftarkan WebSocket + DDS adapter)
 *     -> run_ws_server()          (thread terpisah, non-blocking)
 *     -> run_grpc_server()        (blocking, menunggu request masuk)
 * 
 * Arsitektur:
 *   gRPC Client --> SensorController --> Observer Pattern (log, validate)
 *                                    --> Bridge Pattern (WebSocket, DDS)
 */
#include "utils/log_util/logger.h"
#include "controllers/sensor_controller.h"
#include "websocket/ws_server.h"
#include "dds/dds_publisher.h"
#include "adapters/service_adapters/bridge_manager.h"
#include "adapters/service_adapters/websocket_adapters/websocket_adapter.h"
#include "adapters/service_adapters/dds_adapters/dds_adapter.h"
#include "handlers/sensor_data_log_handler/sensor_data_log_handler.h"
#include "handlers/sensor_data_validator/sensor_data_validator.h"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <thread>
#include <cstdlib>
#include <string>
#include <vector>

/**
 * Anonymous namespace -- variabel global internal untuk file ini saja.
 * Tidak bisa diakses dari file lain (internal linkage).
 * 
 * Variabel ini dibuat global karena digunakan oleh beberapa fungsi:
 *   - g_ws_server : WebSocket server, dipakai oleh run_ws_server() dan init_bridge()
 *   - g_dds_pub   : DDS publisher, dipakai oleh main() dan init_bridge()
 *   - g_bridge    : Bridge manager, dipakai oleh init_bridge() dan run_grpc_server()
 */
namespace {
std::shared_ptr<WsServer> g_ws_server;     // Instance WebSocket server (shared ownership)
std::shared_ptr<DdsPublisher> g_dds_pub;   // Instance DDS publisher (shared ownership)
std::shared_ptr<BridgeManager> g_bridge;   // Instance bridge manager (shared ownership)

/**
 * Ambil nilai environment variable sebagai string.
 * Jika environment variable tidak ada atau kosong, gunakan nilai fallback.
 * 
 * @param name     Nama environment variable (contoh: "GRPC_HOST")
 * @param fallback Nilai default jika env var tidak ditemukan
 * @return         Nilai env var, atau fallback jika tidak ada
 */
std::string get_env_string(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);  // Baca env var dari OS
    if (value && *value) {                  // Pastikan tidak null DAN tidak kosong
        return std::string(value);
    }
    return fallback;                        // Pakai default jika tidak ada
}

/**
 * Ambil nilai environment variable sebagai integer.
 * Jika env var tidak ada, kosong, atau bukan angka valid, gunakan fallback.
 * 
 * @param name     Nama environment variable (contoh: "GRPC_PORT")
 * @param fallback Nilai default jika env var tidak ditemukan atau invalid
 * @return         Nilai env var sebagai int, atau fallback
 */
int get_env_int(const char* name, int fallback) {
    const char* value = std::getenv(name);  // Baca env var dari OS
    if (!value || !*value) {                // Tidak ada atau kosong
        return fallback;
    }
    try {
        return std::stoi(value);            // Coba konversi string ke integer
    } catch (...) {
        return fallback;                    // Jika gagal konversi, pakai default
    }
}

/**
 * Jalankan gRPC Server.
 * 
 * Fungsi ini:
 *   1. Membuat SensorController (yang berperan sebagai gRPC Service + Observable)
 *   2. Mendaftarkan observer: SensorDataLogHandler dan SensorDataValidator
 *   3. Membangun dan menjalankan gRPC server
 * 
 * Server akan BLOCKING di server->Wait() -- artinya fungsi ini tidak return
 * sampai server di-shutdown. Karena itu, WebSocket server harus dijalankan
 * di thread terpisah SEBELUM memanggil fungsi ini.
 * 
 * Environment variables yang digunakan:
 *   - GRPC_HOST : IP address binding (default: "0.0.0.0" = semua interface)
 *   - GRPC_PORT : Port number (default: 50051)
 */
void run_grpc_server() {
    // Baca konfigurasi host dan port dari environment variable
    std::string host = get_env_string("GRPC_HOST", "0.0.0.0");
    int port = get_env_int("GRPC_PORT", 50051);
    std::string server_address = host + ":" + std::to_string(port);

    // Buat SensorController dengan referensi ke BridgeManager
    // SensorController = gRPC Service + Observable (dual role)
    auto service = std::make_shared<SensorController>(g_bridge);

    // Buat concrete observers
    auto log_handler = std::make_shared<SensorDataLogHandler>();  // Observer #1: logging
    auto validator   = std::make_shared<SensorDataValidator>();   // Observer #2: validasi

    // Daftarkan observers ke SensorController (Observable)
    // Setelah ini, setiap data sensor masuk akan otomatis di-log dan di-validasi
    service->add_observer(log_handler);  
    service->add_observer(validator);     

    spdlog::info("Observers: Registered {} handler(s) to SensorController", 2);

    // Build gRPC server dengan konfigurasi
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(service.get());  // Daftarkan service ke gRPC

    // Start server (BLOCKING -- fungsi tidak return sampai server shutdown)
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    spdlog::info("[gRPC] Server active at {}", server_address);
    server->Wait();  // Blocking: tunggu dan handle request sampai shutdown
}

/**
 * Jalankan WebSocket Server.
 * 
 * Fungsi ini dijalankan di thread terpisah (detached) dari main().
 * WebSocket server bertugas menerima koneksi dari browser/client
 * dan mem-broadcast data sensor ke semua client yang terhubung.
 * 
 * Environment variable:
 *   - WS_PORT : Port WebSocket server (default: 9002)
 */
void run_ws_server() {
    int port = get_env_int("WS_PORT", 9002);
    spdlog::info("[WebSocket] Server starting at port {}", port);
    g_ws_server->run(static_cast<uint16_t>(port));  // Blocking di thread sendiri
}

/**
 * Inisialisasi Bridge Manager beserta semua transport adapters.
 * 
 * Bridge Pattern menggabungkan beberapa transport (WebSocket, DDS)
 * ke dalam satu interface yang seragam. Ketika data sensor masuk,
 * BridgeManager cukup memanggil broadcast_sensor_data() dan
 * data akan dikirim ke SEMUA adapter yang terdaftar.
 * 
 * Adapter yang didaftarkan:
 *   1. WebSocketAdapter -- kirim data ke browser/frontend via WebSocket
 *   2. DdsAdapter       -- kirim data ke DDS network via OpenDDS
 */
void init_bridge() {
    g_bridge = std::make_shared<BridgeManager>();
    
    // Buat adapter dengan referensi ke server/publisher masing-masing
    auto ws_adapter = std::make_shared<WebSocketAdapter>(g_ws_server);   // Adapter WebSocket
    auto dds_adapter = std::make_shared<DdsAdapter>(g_dds_pub);         // Adapter DDS
    
    // Daftarkan kedua adapter ke bridge
    g_bridge->add_adapter(ws_adapter);
    g_bridge->add_adapter(dds_adapter);
    
    spdlog::info("Bridge: Initialized with {} adapters", 2);
}
}  // end anonymous namespace

/**
 * main() -- Titik masuk utama aplikasi.
 * 
 * Urutan eksekusi:
 *   1. Inisialisasi logger (spdlog)
 *   2. Buat instance WebSocket server dan DDS publisher
 *   3. Siapkan argumen DDS config file (-DCPSConfigFile)
 *   4. Inisialisasi DDS publisher (koneksi ke DDS network)
 *   5. Inisialisasi bridge (daftarkan WebSocket + DDS adapter)
 *   6. Jalankan WebSocket server di thread terpisah (non-blocking)
 *   7. Jalankan gRPC server di main thread (blocking)
 * 
 * @param argc Jumlah argumen command line
 * @param argv Array argumen command line
 * @return 0 jika sukses, 1 jika DDS init gagal
 */
int main(int argc, char* argv[]) {
    // 1. Setup logging -- harus pertama agar semua log berikutnya tercatat
    utils::init_logger();
    
    // 2. Buat instance server (belum dijalankan, hanya alokasi objek)
    g_ws_server = std::make_shared<WsServer>();
    g_dds_pub = std::make_shared<DdsPublisher>();
    
    // 3. Susun argumen untuk DDS -- tambahkan flag -DCPSConfigFile
    //    agar OpenDDS tahu file konfigurasi RTPS mana yang dipakai
    std::string config_path = get_env_string("DDS_CONFIG_FILE", "rtps.ini");
    std::vector<std::string> arg_strings;
    std::vector<char*> new_argv;
    for (int i = 0; i < argc; ++i) {
        arg_strings.push_back(argv[i]);        // Copy argumen asli
    }
    arg_strings.push_back("-DCPSConfigFile");  // Flag OpenDDS
    arg_strings.push_back(config_path);         // Path file konfigurasi
    for (auto& s : arg_strings) {
        new_argv.push_back(&s[0]);              // Konversi ke char* array
    }
    new_argv.push_back(nullptr);                // Null terminator (standar C)
    int new_argc = static_cast<int>(new_argv.size() - 1);

    // 4. Inisialisasi DDS Publisher -- jika gagal, aplikasi berhenti
    if (!g_dds_pub->init(new_argc, new_argv.data())) {
        spdlog::error("Failed to initialize DDS Publisher");
        return 1;  // Exit dengan error code
    }
    
    // 5. Setup bridge (WebSocket adapter + DDS adapter)
    init_bridge();
    
    // 6. Jalankan WebSocket server di thread terpisah
    //    detach() artinya thread berjalan independen, tidak perlu di-join
    std::thread ws_thread(run_ws_server);
    ws_thread.detach();

    // 7. Jalankan gRPC server (BLOCKING -- program berhenti di sini sampai shutdown)
    run_grpc_server();

    return 0;
}