/**
 * sensor_controller.h -- Definisi class SensorController
 *
 * File ini mendefinisikan SensorController yang menggabungkan dua peran:
 * gRPC Service (menerima request dari client) dan Observable (memberitahu
 * observer tentang data baru). Ini adalah titik masuk utama data sensor
 * dari sisi jaringan gRPC ke seluruh sistem internal.
 */
#pragma once
#include "sensor.grpc.pb.h"
#include "adapters/abstract_adapters/observable/observable.h"
#include <grpcpp/grpcpp.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>
#include <memory>

// Forward declaration -- cukup deklarasi nama class saja karena header
// ini hanya menggunakan pointer/reference ke BridgeManager, bukan objek langsung.
class BridgeManager;

/**
 * SensorController -- gRPC Service + Observable
 *
 * Class ini menggabungkan DUA peran:
 *   1. gRPC Service  (extends SensorService::Service) -- handle RPC calls
 *   2. Observable     (extends Observable)             -- notify observers
 *
 * Alur data:
 *   gRPC Request masuk
 *       -> SensorController menerima
 *           -> notify_observers(request)    <-- Observer Pattern (log, validate, dll)
 *           -> bridge_->broadcast(request)  <-- Bridge Pattern (WebSocket, DDS)
 *
 * Dengan ini, SensorController TIDAK perlu tahu:
 *   - Berapa banyak observer yang terdaftar
 *   - Apa yang dilakukan setiap observer
 *   - Observer bisa ditambah/dihapus tanpa mengubah class ini
 */
class SensorController final : public iot::SensorService::Service, public Observable {
public:
    // Constructor menerima shared_ptr ke BridgeManager
    // agar SensorController bisa meneruskan data ke semua transport adapter
    explicit SensorController(std::shared_ptr<BridgeManager> bridge);

    /**
     * Unary RPC -- Client kirim 1 request, server balas 1 response.
     * Pola paling sederhana. Cocok untuk pengiriman data sensor sekali kirim.
     */
    grpc::Status SendSensorData(grpc::ServerContext* context, 
                               const iot::SensorRequest* request, 
                               iot::SensorResponse* response) override;

    /**
     * Client Streaming RPC -- Client kirim banyak request secara berurutan,
     * server baca semuanya lalu balas 1 response di akhir.
     * Cocok untuk batch upload data sensor.
     */
    grpc::Status StreamSensorData(grpc::ServerContext* context,
                                 grpc::ServerReader<iot::SensorRequest>* reader,
                                 iot::SensorResponse* response) override;

    /**
     * Server Streaming RPC -- Client kirim 1 request, server balas
     * banyak response secara berkala (setiap 2 detik).
     * Cocok untuk monitoring sensor secara real-time dari sisi client.
     */
    grpc::Status MonitorSensor(grpc::ServerContext* context,
                              const iot::SensorRequest* request,
                              grpc::ServerWriter<iot::SensorResponse>* writer) override;

    /**
     * Bidirectional Streaming RPC -- Client dan server saling kirim
     * data secara bersamaan. Keduanya bisa baca dan tulis kapan saja.
     * Cocok untuk skenario interaktif seperti command-response sensor.
     */
    grpc::Status InteractiveSensor(grpc::ServerContext* context,
                                  grpc::ServerReaderWriter<iot::SensorResponse, iot::SensorRequest>* stream) override;

private:
    // Pointer ke BridgeManager -- digunakan untuk broadcast data sensor
    // ke semua transport adapter (WebSocket, DDS, dll.) yang terdaftar
    std::shared_ptr<BridgeManager> bridge_;
};
