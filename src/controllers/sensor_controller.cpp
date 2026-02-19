/**
 * sensor_controller.cpp -- Implementasi SensorController
 * 
 * File ini berisi implementasi 4 jenis gRPC communication pattern:
 *   1. Unary RPC          (SendSensorData)    : client kirim 1, server balas 1
 *   2. Client Streaming   (StreamSensorData)  : client kirim banyak, server balas 1
 *   3. Server Streaming   (MonitorSensor)      : client kirim 1, server balas banyak
 *   4. Bidirectional      (InteractiveSensor)  : client dan server saling kirim
 * 
 * Setiap method yang menerima data sensor menjalankan dua aksi:
 *   a. Observer Pattern  : notify_observers() -> log, validasi, dll
 *   b. Bridge Pattern    : bridge_->broadcast_sensor_data() -> WebSocket, DDS
 * 
 * Pemisahan ini membuat SensorController tetap "tipis" (thin controller).
 * Logic logging, validasi, dan transport ada di class lain.
 */
#include "sensor_controller.h"
#include "adapters/service_adapters/bridge_manager.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>

/**
 * Constructor: simpan referensi ke BridgeManager.
 * BridgeManager digunakan untuk broadcast data ke semua transport adapter.
 * 
 * @param bridge Shared pointer ke BridgeManager (bisa null jika bridge belum siap)
 */
SensorController::SensorController(std::shared_ptr<BridgeManager> bridge)
    : bridge_(bridge) {
}

/**
 * Unary RPC -- Client kirim 1 request, server balas 1 response.
 * 
 * Ini adalah pola paling sederhana dalam gRPC.
 * Cocok untuk pengiriman data sensor satu per satu (single reading).
 * 
 * Alur:
 *   1. Client mengirim SensorRequest (1 data sensor)
 *   2. Server menerima request
 *   3. notify_observers() -> semua observer bereaksi (log, validasi)
 *   4. bridge_->broadcast() -> kirim ke WebSocket + DDS
 *   5. Server mengirim SensorResponse (success/fail)
 * 
 * @param context  Konteks gRPC (metadata, deadline, cancellation)
 * @param request  Data sensor dari client (protobuf)
 * @param response Response yang akan dikirim balik ke client
 * @return Status::OK jika berhasil
 */
grpc::Status SensorController::SendSensorData(grpc::ServerContext* context, 
                                             const iot::SensorRequest* request, 
                                             iot::SensorResponse* response) {
    spdlog::info("Incoming gRPC -> Sensor ID: {}, Temp: {}C", 
                 request->sensor_id(), request->temperature());
    
    // OBSERVER PATTERN: Notify semua observer (LogHandler, Validator, dll)
    // Observer bereaksi SEBELUM data dikirim ke transport adapters.
    // Ini memungkinkan validasi/logging tanpa mengubah code di sini.
    notify_observers(*request);
    
    // BRIDGE PATTERN: Broadcast ke semua transport adapters (WebSocket, DDS)
    if (bridge_) {
        bridge_->broadcast_sensor_data(*request);
    }
    
    // Kirim response sukses ke client
    response->set_success(true);
    response->set_message("Bridge: Data processed successfully");
    return grpc::Status::OK;
}

/**
 * Client Streaming RPC -- Client kirim banyak request, server balas 1 response.
 * 
 * Client mengirim stream (aliran) data sensor secara berurutan.
 * Server membaca satu per satu sampai client selesai (stream ditutup),
 * kemudian mengirim satu response rangkuman.
 * 
 * Cocok untuk:
 *   - Batch upload data sensor
 *   - Client yang mengumpulkan banyak data sebelum mengirim
 * 
 * Alur:
 *   Client: Send(req1) -> Send(req2) -> ... -> Send(reqN) -> CloseStream
 *   Server: Read(req1), Read(req2), ..., Read(reqN), SendResponse
 * 
 * @param context  Konteks gRPC
 * @param reader   Stream reader untuk membaca request dari client
 * @param response Response tunggal yang dikirim setelah semua data dibaca
 * @return Status::OK jika berhasil
 */
grpc::Status SensorController::StreamSensorData(
    grpc::ServerContext* context,
    grpc::ServerReader<iot::SensorRequest>* reader,
    iot::SensorResponse* response) {
    (void)context;  // Suppress unused parameter warning

    iot::SensorRequest request;
    int count = 0;  // Counter jumlah message yang diterima

    // Baca request satu per satu dari stream client
    // Loop berhenti saat client menutup stream (reader->Read() return false)
    while (reader->Read(&request)) {
        spdlog::info("[Stream] Sensor ID: {}, Temp: {}C", request.sensor_id(), request.temperature());

        // OBSERVER: Notify untuk setiap message dalam stream
        notify_observers(request);

        // BRIDGE: Broadcast ke adapters
        if (bridge_) {
            bridge_->broadcast_sensor_data(request);
        }
        ++count;
    }

    // Kirim response rangkuman setelah semua data dibaca
    response->set_success(true);
    response->set_message("Bridge: Stream processed");
    response->set_processed_timestamp(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());  // Timestamp saat processing selesai

    spdlog::info("[Stream] Total messages received: {}", count);
    return grpc::Status::OK;
}

/**
 * Server Streaming RPC -- Client kirim 1 request, server balas banyak response.
 * 
 * Client mengirim satu request (berisi info sensor yang ingin dimonitor),
 * lalu server mengirim stream response secara berkala (setiap 1 detik).
 * 
 * Cocok untuk:
 *   - Real-time monitoring satu sensor tertentu
 *   - Dashboard yang menampilkan update berkala
 * 
 * Alur:
 *   Client: Send(request) -> Receive(res1), Receive(res2), ..., Receive(res5)
 *   Server: Kirim 5 update dengan interval 1 detik
 * 
 * Catatan: Saat ini menggunakan data dummy (5 update fixed).
 * Untuk production, bisa diganti dengan data real-time dari sensor.
 * 
 * @param context  Konteks gRPC (bisa digunakan untuk cek cancellation)
 * @param request  Request dari client (sensor mana yang mau dimonitor)
 * @param writer   Stream writer untuk mengirim response ke client
 * @return Status::OK jika selesai, Status::CANCELLED jika client cancel
 */
grpc::Status SensorController::MonitorSensor(
    grpc::ServerContext* context,
    const iot::SensorRequest* request,
    grpc::ServerWriter<iot::SensorResponse>* writer) {
    
    spdlog::info("[Monitor] Starting stream for Sensor ID: {}", request->sensor_id());

    // Kirim 5 update ke client dengan interval 1 detik
    for (int i = 1; i <= 5; ++i) {
        // Cek apakah client membatalkan request (misalnya disconnect)
        if (context->IsCancelled()) {
            return grpc::Status::CANCELLED;  // Hentikan stream jika client cancel
        }

        // Buat response untuk setiap update
        iot::SensorResponse res;
        res.set_success(true);
        res.set_message("Real-time update #" + std::to_string(i));
        res.set_processed_timestamp(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());

        writer->Write(res);  // Kirim response ke client
        spdlog::info("[Monitor] Sent update #{} to client", i);

        // Tunggu 1 detik sebelum mengirim update berikutnya
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return grpc::Status::OK;
}

/**
 * Bidirectional Streaming RPC -- Client dan server saling kirim secara bersamaan.
 * 
 * Ini adalah pola paling kompleks dalam gRPC.
 * Client dan server bisa mengirim dan menerima data secara independen.
 * 
 * Cocok untuk:
 *   - Sesi interaktif (client kirim data, server langsung merespons)
 *   - Chat-style communication antara sensor dan server
 *   - Scenario di mana server perlu merespons setiap data yang masuk
 * 
 * Alur:
 *   Client: Send(req1) -----> Send(req2) -----> CloseStream
 *   Server: <--- Recv(req1), Write(res1) <--- Recv(req2), Write(res2)
 * 
 * @param context  Konteks gRPC
 * @param stream   Bidirectional stream (bisa read DAN write)
 * @return Status::OK setelah client menutup stream
 */
grpc::Status SensorController::InteractiveSensor(
    grpc::ServerContext* context,
    grpc::ServerReaderWriter<iot::SensorResponse, iot::SensorRequest>* stream) {
    
    iot::SensorRequest request;
    spdlog::info("[Interactive] Session started");

    // Baca request satu per satu dari client
    // Untuk setiap request, langsung kirim response balik
    while (stream->Read(&request)) {
        spdlog::info("[Interactive] Received Sensor ID: {} from {}", request.sensor_id(), request.location());

        // OBSERVER: Notify untuk setiap message dalam bidirectional stream
        notify_observers(request);

        // BRIDGE: Broadcast ke adapters
        if (bridge_) {
            bridge_->broadcast_sensor_data(request);
        }

        // Buat dan kirim response echo untuk setiap request yang diterima
        iot::SensorResponse response;
        response.set_success(true);
        response.set_message("Server Echo: Received data from " + request.location());
        response.set_processed_timestamp(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());

        stream->Write(response);  // Kirim response balik ke client
    }

    spdlog::info("[Interactive] Session closed by client");
    return grpc::Status::OK;
}
