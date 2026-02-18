#pragma once
#include "sensor.grpc.pb.h"
#include "adapters/abstract_adapters/observable/observable.h"
#include <grpcpp/grpcpp.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>
#include <memory>

class BridgeManager;

/**
 * SensorController — gRPC Service + Observable
 * 
 * Class ini menggabungkan DUA peran:
 *   1. gRPC Service  (extends SensorService::Service) → handle RPC calls
 *   2. Observable     (extends Observable)             → notify observers
 * 
 * Alur data:
 *   gRPC Request masuk
 *       → SensorController menerima
 *           → notify_observers(request)    ← Observer Pattern (log, validate, dll)
 *           → bridge_->broadcast(request)  ← Bridge Pattern (WebSocket, DDS)
 * 
 * Dengan ini, SensorController TIDAK perlu tahu:
 *   - Berapa banyak observer yang terdaftar
 *   - Apa yang dilakukan setiap observer
 *   - Observer bisa ditambah/dihapus tanpa mengubah class ini
 */
class SensorController final : public iot::SensorService::Service, public Observable {
public:
    explicit SensorController(std::shared_ptr<BridgeManager> bridge);

    grpc::Status SendSensorData(grpc::ServerContext* context, 
                               const iot::SensorRequest* request, 
                               iot::SensorResponse* response) override;

    grpc::Status StreamSensorData(grpc::ServerContext* context,
                                 grpc::ServerReader<iot::SensorRequest>* reader,
                                 iot::SensorResponse* response) override;

    grpc::Status MonitorSensor(grpc::ServerContext* context,
                              const iot::SensorRequest* request,
                              grpc::ServerWriter<iot::SensorResponse>* writer) override;

    grpc::Status InteractiveSensor(grpc::ServerContext* context,
                                  grpc::ServerReaderWriter<iot::SensorResponse, iot::SensorRequest>* stream) override;

private:
    std::shared_ptr<BridgeManager> bridge_;
};
