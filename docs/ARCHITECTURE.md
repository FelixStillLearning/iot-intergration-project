# IoT Integration Project - Architecture Guide

## 📚 Daftar Isi
1. [Alur Data Keseluruhan](#alur-data-keseluruhan)
2. [Design Pattern yang Digunakan](#design-pattern-yang-digunakan)
3. [Struktur Folder](#struktur-folder)
4. [Penjelasan Komponen](#penjelasan-komponen)
5. [Alur Data Detail](#alur-data-detail)
6. [Cara Memahami Kode](#cara-memahami-kode)
7. [Contoh Request-Response](#contoh-request-response)

---

## 🔄 Alur Data Keseluruhan

```
┌─────────────────────────────────────────────────────────────┐
│                      CLIENT LAYER                           │
│  (BloomRPC / Python Script / Browser)                       │
└──────────────────────────┬──────────────────────────────────┘
                           │
                    (gRPC Request)
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                  SENSOR CONTROLLER                          │
│  - Terima data sensor via gRPC                             │
│  - Notify observers (log, validate)                        │
│  - Forward ke bridge untuk transport                       │
└──────────────────────────┬──────────────────────────────────┘
                           │
                ┌──────────┴──────────┐
                │                     │
         (Observer)            (Bridge Pattern)
                │                     │
    ┌───────────▼────────────┐    ┌───▼──────────────────┐
    │ Observers              │    │ BridgeManager        │
    ├───────────────────────┤    ├──────────────────────┤
    │ 1. LogHandler         │    │ ┌──────────────────┐ │
    │    └─ Log data        │    │ │WebSocketAdapter  │ │
    │                       │    │ │ └─ Kirim ke WS   │ │
    │ 2. Validator         │    │ │                   │ │
    │    └─ Validate data   │    │ └──────────────────┘ │
    │                       │    │                      │
    │ 3. Custom Observer?  │    │ ┌──────────────────┐ │
    │    └─ Your logic      │    │ │DdsAdapter        │ │
    │                       │    │ │ └─ Kirim ke DDS  │ │
    └───────────────────────┘    │ │                   │
                                 │ └──────────────────┘ │
                                 └──────────────────────┘
                           │                     │
                    ┌──────▼─────┐         ┌────▼────────┐
                    │  WebSocket  │       │  OpenDDS     │
                    │   Server    │       │  Network     │
                    │             │       │              │
                    │ Broadcast   │       │ Publish      │
                    │ to Browser  │       │ to Nodes     │
                    └─────────────┘       └──────────────┘
```

---

## 🎨 Design Pattern yang Digunakan

### 1. **Observer Pattern** (Behavioral)

**Tujuan:** Membuat data sensor bisa "diamati" oleh multiple observers tanpa perlu tahu
siapa saja yang mengamati.

**Yang ada di project:**
- **Observable** (Subject) → `src/adapters/abstract_adapters/observable/observable.h`
- **Observer** (Interface) → `src/handlers/observer/observer.h`
- **Concrete Observers:**
  - `SensorDataLogHandler` → Log setiap data sensor
  - `SensorDataValidator` → Validasi data sensor
  - Bisa tambah observer baru tanpa ubah SensorController!

**Alur:**
```
main() 
  ▼
SensorController (Observable)
  ├─ add_observer(LogHandler)
  ├─ add_observer(Validator)
  └─ add_observer(CustomObserver)
  
Data sensor masuk
  ▼
SensorController.notify_observers(request)
  ├─ LogHandler.on_sensor_data(request)     ✓
  ├─ Validator.on_sensor_data(request)      ✓
  └─ CustomObserver.on_sensor_data(request) ✓
```

**Keuntungan:**
- ✅ Loose coupling: SensorController tidak tahu siapa observers-nya
- ✅ Easy to extend: Tambah observer baru tanpa ubah existing code
- ✅ Single Responsibility: LogHandler hanya log, Validator hanya validate

---

### 2. **Bridge Pattern** (Structural)

**Tujuan:** Memisahkan logic "apa yang mau dilakukan" dari "bagaimana caranya".

**Yang ada di project:**
- **IBridgeManager** (Abstraction) → Kontrak interface
- **BridgeManager** (Concrete) → Mengelola adapters
- **ITransportAdapter** (Interface) → Kontrak untuk adapters
- **Concrete Adapters:**
  - `WebSocketAdapter` → Kirim via WebSocket
  - `DdsAdapter` → Kirim via DDS
  - Bisa tambah adapter baru (misalnya MQTT, Kafka, HTTP)

**Alur:**
```
BridgeManager
  ├─ add_adapter(WebSocketAdapter)
  ├─ add_adapter(DdsAdapter)
  └─ add_adapter(MqttAdapter) ← Bisa tambah baru!

broadcast_sensor_data(request)
  ├─ adapter[0].send(request)  // WebSocket
  ├─ adapter[1].send(request)  // DDS
  └─ adapter[2].send(request)  // MQTT (jika ditambahkan)
```

**Keuntungan:**
- ✅ Transport bisa di-swap tanpa ubah business logic
- ✅ Mudah add/remove transport method
- ✅ Setiap adapter bertanggung jawab sendiri untuk proses pengiriman

---

### 3. **Adapter Pattern** (Structural)

**Tujuan:** Membuat interface tidak kompatibel menjadi kompatibel.

**Di project ini:**
- `WebSocketAdapter` → Mengadaptasi `SensorRequest` (protobuf) ke JSON
- `DdsAdapter` → Mengadaptasi `SensorRequest` ke IDL message

**Contoh WebSocketAdapter:**
```cpp
void WebSocketAdapter::send(const iot::SensorRequest& request) {
    // Adaptasi: convert Protobuf → JSON
    std::string json = sensor_to_json(&request);
    
    // Gunakan WebSocket API
    ws_server_->broadcast(json);
}
```

---

## 📁 Struktur Folder

```
src/
├── app.cpp                              ← Entry point, setup semua component
│
├── controllers/
│   ├── sensor_controller.h
│   └── sensor_controller.cpp            ← gRPC service + Observable
│
├── adapters/
│   ├── abstract_adapters/
│   │   └── observable/
│   │       ├── interface_observable.h   ← Pure virtual interface
│   │       ├── observable.h             ← Base implementation
│   │       └── observable.cpp
│   │
│   ├── interface_adapters/
│   │   ├── interface_bridge_manager.h   ← Bridge abstraction
│   │   └── interface_transport_adapter.h ← Transport abstraction
│   │
│   └── service_adapters/
│       ├── bridge_manager.h
│       ├── bridge_manager.cpp           ← Concrete bridge
│       ├── websocket_adapters/
│       │   ├── websocket_adapter.h
│       │   └── websocket_adapter.cpp    ← Adapter untuk WebSocket
│       └── dds_adapters/
│           ├── dds_adapter.h
│           └── dds_adapter.cpp          ← Adapter untuk DDS
│
├── handlers/                             ← Observer implementations
│   ├── observer/
│   │   └── observer.h                   ← Observer interface
│   ├── sensor_data_log_handler/
│   │   ├── sensor_data_log_handler.h
│   │   └── sensor_data_log_handler.cpp  ← Observer #1: Logging
│   └── sensor_data_validator/
│       ├── sensor_data_validator.h
│       └── sensor_data_validator.cpp    ← Observer #2: Validation
│
├── websocket/
│   ├── ws_server.h
│   └── ws_server.cpp                    ← WebSocket server
│
├── dds/
│   ├── dds_publisher.h
│   └── dds_publisher.cpp                ← DDS publisher
│
├── models/
│   ├── proto_models/                    ← Generated Protobuf models
│   └── odds_models/                     ← Generated DDS models
│
└── utils/
    ├── json_helper.h                    ← Convert Protobuf to JSON
    └── log_util/
        └── logger.h                     ← Logging utility
```

---

## 🔧 Penjelasan Komponen

### 1. **SensorController** (`src/controllers/sensor_controller.h`)
```
Adalah:
  - gRPC Service (menerima RPC calls dari client)
  - Observable (mengelola observers)

Tanggung jawab:
  1. Terima gRPC request dari client
  2. Notify semua observers (log, validate, dll)
  3. Forward ke bridge untuk kirim ke transports (WebSocket, DDS)

Contoh alur:
  client → gRPC SendSensorData()
       ↓
  SensorController.SendSensorData()
       ↓
  notify_observers(request)  ← Observers process data
       ↓
  bridge_->broadcast(request) ← Kirim ke WebSocket + DDS
       ↓
  response dikirim balik ke client
```

### 2. **BridgeManager** (`src/adapters/service_adapters/bridge_manager.h`)
```
Adalah:
  - Pengelola transport adapters
  - Bertugas mendistribusikan data ke semua transports

Tanggung jawab:
  1. Menyimpan daftar adapters (WebSocket, DDS, dll)
  2. Memanggil send() pada semua adapters saat broadcast

Contoh:
  adapters = [WebSocketAdapter, DdsAdapter]
  
  broadcast_sensor_data(request)
    for adapter in adapters:
      adapter.send(request)
```

### 3. **WebSocketAdapter** (`src/adapters/service_adapters/websocket_adapters/websocket_adapter.h`)
```
Adalah:
  - Concrete adapter untuk WebSocket transport
  - Menjembatani SensorController ke WsServer

Alur:
  BridgeManager.broadcast()
       ↓
  WebSocketAdapter.send(request)
       ↓
  Convert Protobuf → JSON
       ↓
  WsServer.broadcast(json)
       ↓
  Kirim ke semua browser clients
```

### 4. **DdsAdapter** (`src/adapters/service_adapters/dds_adapters/dds_adapter.h`)
```
Adalah:
  - Concrete adapter untuk DDS transport
  - Menjembatani SensorController ke DdsPublisher

Alur:
  BridgeManager.broadcast()
       ↓
  DdsAdapter.send(request)
       ↓
  Extract fields dari Protobuf
       ↓
  DdsPublisher.publish(id, name, temp, ...)
       ↓
  Publish ke DDS network via OpenDDS
```

### 5. **Observers**
```
SensorDataLogHandler:
  - Dipanggil saat sensor data masuk
  - Tidak mengubah data, cuma log
  - Tidak blocking (instant notify)

SensorDataValidator:
  - Dipanggil saat sensor data masuk
  - Validasi range temperature, humidity, dll
  - Alert ke log jika ada anomali
  - Tidak mengubah flow (data tetap diteruskan)

Custom Observer (Bisa ditambah):
  - Simpan ke database
  - Send email alert
  - Trigger automation
  - Etc.
```

---

## 📊 Alur Data Detail (Step-by-Step)

### Scenario: Client kirim sensor data via gRPC

```
STEP 1: CLIENT mengirim request
=============================
Client (BloomRPC / Python)
  │
  └─ gRPC.SendSensorData({
       sensor_id: 1,
       sensor_name: "Sensor-A",
       temperature: 25.5,
       humidity: 60.0,
       pressure: 1013.2,
       light_intensity: 300.5,
       timestamp: 1708338005,
       location: "Lab"
     })

STEP 2: gRPC SERVER menerima
=============================
main()
  ├─ init DDS Publisher
  ├─ init WebSocket Server (thread)
  ├─ init SensorController
  │   ├─ add_observer(LogHandler)
  │   ├─ add_observer(Validator)
  │   └─ set bridge_manager
  └─ run gRPC Server (blocking)
  
  gRPC request masuk
    └─ SensorController::SendSensorData() dipanggil

STEP 3: NOTIFY OBSERVERS
========================
SensorController::SendSensorData(context, request, response)
  {
    1. notify_observers(request)
       ├─ LogHandler::on_sensor_data(request)
       │   └─ spdlog::info("[DataLogger] Sensor ID: 1, Temp: 25.5°C, ...")
       │
       └─ Validator::on_sensor_data(request)
           ├─ check sensor_id > 0 ✓
           ├─ check temp in range [-50, 100] ✓
           ├─ check humidity in range [0, 100] ✓
           ├─ check pressure in range [300, 1100] ✓
           └─ spdlog::info("[Validator] ✓ Data VALID")
  
    2. bridge_->broadcast_sensor_data(request)
       └─ BridgeManager::broadcast_sensor_data(request)
          │
          ├─ adapters[0]->send(request)  // WebSocketAdapter
          │   ├─ Convert SensorRequest → JSON
          │   │   {
          │   │     "sensor_id": 1,
          │   │     "temperature": 25.5,
          │   │     "humidity": 60.0,
          │   │     "location": "Lab"
          │   │   }
          │   │
          │   └─ ws_server_->broadcast(json)
          │       ├─ for each client connection:
          │       │   └─ send json_string
          │       │
          │       └─ Browser/Dashboard terima data JSON
          │           └─ Update chart/UI real-time ✓
          │
          └─ adapters[1]->send(request)  // DdsAdapter
              ├─ Extract fields: id=1, name="Sensor-A", temp=25.5, ...
              │
              └─ dds_publisher_->publish(id, name, temp, hum, press, light, ts, loc)
                  ├─ Create DDS Message (Messengger::Message)
                  ├─ DataWriter.write(message)
                  │
                  └─ OpenDDS Network (RTPS protocol)
                      └─ Other nodes dalam DDS domain terima data ✓
    
    3. Prepare response & send back
       response.set_success(true);
       response.set_message("Data received and processed");
       └─ gRPC send response ke client ✓
  }

STEP 4: RESULT
==============
✅ Log console: 
   [DataLogger] Sensor ID: 1, Temp: 25.5°C, Humidity: 60%
   [Validator] ✓ Data VALID

✅ Browser dashboard: Update chart dengan data sensor real-time

✅ DDS Network: Node lain bisa subscribe ke data

✅ Client gRPC: Terima response "Data received and processed"
```

---

## 🎓 Cara Memahami Kode

### Langkah 1: Mulai dari Entry Point
```
File: src/app.cpp, fungsi main()

Pahami:
1. init_logger()           → Setup logging
2. DdsPublisher::init()    → Setup DDS
3. WsServer: run()         → Start WebSocket (thread)
4. init_bridge()           → Setup adapters
5. run_grpc_server()       → Start gRPC (blocking)
```

### Langkah 2: Pahami SensorController
```
File: src/controllers/sensor_controller.h & sensor_controller.cpp

Pahami:
1. SendSensorData() - method utama yang dapat gRPC request
2. notify_observers() - dari Observable base class
3. bridge_->broadcast_sensor_data() - forward ke adapters
```

### Langkah 3: Pahami Observer Pattern
```
File: src/handlers/observer/observer.h
File: src/handlers/sensor_data_log_handler/sensor_data_log_handler.h
File: src/handlers/sensor_data_validator/sensor_data_validator.h

Pahami:
1. Observer adalah interface dengan method on_sensor_data()
2. LogHandler dan Validator implement Observer
3. Keduanya auto-dipanggil saat data masuk
```

### Langkah 4: Pahami Bridge Pattern
```
File: src/adapters/service_adapters/bridge_manager.h
File: src/adapters/interface_adapters/interface_transport_adapter.h

Pahami:
1. BridgeManager mengelola adapters
2. Setiap adapter implement send()
3. broadcast() memanggil send() di semua adapters
```

### Langkah 5: Pahami Concrete Adapters
```
File: src/adapters/service_adapters/websocket_adapters/websocket_adapter.h
File: src/adapters/service_adapters/dds_adapters/dds_adapter.h

Pahami:
1. WebSocketAdapter convert protobuf → JSON
2. DdsAdapter convert protobuf → IDL
3. Keduanya implement ITransportAdapter::send()
```

---

## 💡 Contoh Request-Response

### Scenario 1: gRPC Unary (RequestResponse sederhana)

```
┌─ CLIENT SIDE (Python Client) ─────────────────────────┐
│                                                         │
│  import grpc                                            │
│  from sensor_pb2_grpc import SensorServiceStub         │
│  from sensor_pb2 import SensorRequest                  │
│                                                         │
│  channel = grpc.aio.secure_channel("localhost:50051")  │
│  stub = SensorServiceStub(channel)                     │
│                                                         │
│  request = SensorRequest(                              │
│      sensor_id=1,                                      │
│      sensor_name="Sensor-A",                           │
│      temperature=25.5,                                 │
│      humidity=60.0,                                    │
│      pressure=1013.2,                                  │
│      light_intensity=300.5,                            │
│      timestamp=1708338005,                             │
│      location="Lab"                                    │
│  )                                                      │
│                                                         │
│  response = stub.SendSensorData(request)               │
│  print(response.success)        # True                 │
│  print(response.message)        # "Data processed"     │
│                                                         │
└─────────────────────────────────────────────────────────┘
                          ▼
                    (gRPC Request)
                          ▼
┌─ SERVER SIDE (C++ Server) ─────────────────────────────┐
│                                                         │
│  SensorController::SendSensorData()                    │
│  {                                                      │
│    1. Log: "[DataLogger] Sensor ID: 1, Temp: 25.5°C"  │
│    2. Validate: "[Validator] ✓ Data VALID"            │
│    3. Broadcast:                                       │
│       - WebSocket: Send JSON to browser                │
│       - DDS: Publish to OpenDDS network                │
│    4. Return response                                  │
│  }                                                      │
│                                                         │
└─────────────────────────────────────────────────────────┘
                          ▼
                    (gRPC Response)
                          ▼
┌─ CLIENT RESPONSE ─────────────────────────────────────┐
│                                                         │
│  response.success = true                               │
│  response.message = "Data received and processed"      │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### Scenario 2: WebSocket Real-Time Data

```
┌─ BROWSER CLIENT ──────────────────────────────────────┐
│                                                        │
│  <script>                                             │
│    const ws = new WebSocket("ws://localhost:9002");   │
│    ws.onmessage = (event) => {                        │
│      const data = JSON.parse(event.data);            │
│      console.log("Sensor: " + data.sensor_id);       │
│      updateChart(data.temperature);                   │
│    };                                                 │
│  </script>                                            │
│                                                        │
└────────────────────────────────────────────────────────┘
                          ▲
                    (WebSocket)
                          │
             ┌────────────┴──────────────┐
             │                           │
      [Broadcast JSON]         [Every time gRPC]
             │                   [request received]
   {"sensor_id": 1,
    "temperature": 25.5,
    "humidity": 60.0,
    "location": "Lab"}
```

### Scenario 3: DDS Network Communication

```
┌─ IoT Bridge (Publisher) ──────────────────────────────┐
│                                                       │
│  DdsAdapter::send(request)                           │
│    └─ DdsPublisher::publish(...)                     │
│        └─ DataWriter.write(Messengger::Message)      │
│            └─ RTPS Protocol                           │
│                └─ UDP Multicast                       │
│                                                       │
└───────────────────────────────────────────────────────┘
                      ▼
            ┌─────────────────┐
            │  DDS Domain 0   │
            │  (Network)      │
            └─────────────────┘
                      ▲
        ┌─────────────┴──────────────┐
        │                            │
┌─ Subscriber Node A ──────┐  ┌─ Subscriber Node B ──────┐
│                          │  │                          │
│ DataReader.take()        │  │ DataReader.take()        │
│   └─ Receive:           │  │   └─ Receive:           │
│     # Sensor #1         │  │     # Sensor #1         │
│     ID: 1               │  │     ID: 1               │
│     Temp: 25.5°C        │  │     Temp: 25.5°C        │
│     Location: Lab       │  │     Location: Lab       │
│                          │  │                          │
└──────────────────────────┘  └──────────────────────────┘
```

---

## 🔍 Quick Reference: Yang Terjadi Saat Request Masuk

```
Request dari gRPC Client
        │
        ▼
gRPC Server menerima & call
SensorController::SendSensorData()
        │
        ├─→ [OBSERVER PATTERN]
        │   notify_observers(request)
        │   │
        │   ├─→ SensorDataLogHandler::on_sensor_data()
        │   │   └─ Log ke console
        │   │
        │   └─→ SensorDataValidator::on_sensor_data()
        │       └─ Check range temperature, humidity, dll
        │
        └─→ [BRIDGE PATTERN]
            bridge_->broadcast_sensor_data(request)
            │
            ├─→ [ADAPTER PATTERN #1]
            │   WebSocketAdapter::send(request)
            │   └─ Convert JSON & broadcast ke browser
            │
            └─→ [ADAPTER PATTERN #2]
                DdsAdapter::send(request)
                └─ Publish ke DDS network
        │
        ▼
Response dikirim balik ke client
```

---

## 🎯 Kesimpulan

**Tiga Pattern Utama:**

1. **Observer Pattern** → Decouple observers dari SensorController
   - Bisa add lebih banyak observers (log, validate, email, db, dll)
   - Tanpa mengubah SensorController

2. **Bridge Pattern** → Decouple transport abstraction dari implementasi
   - Bisa swap transport (WebSocket ↔ DDS ↔ MQTT)
   - Tanpa mengubah SensorController

3. **Adapter Pattern** → Adapt format data untuk setiap transport
   - WebSocket: Protobuf → JSON
   - DDS: Protobuf → IDL

**Hasilnya: Flexible, Maintainable, Extensible System! 🎉**

---

## 📖 Referensi File Penting

| File | Fungsi |
|------|--------|
| `src/app.cpp` | Entry point, setup component |
| `src/controllers/sensor_controller.h/.cpp` | gRPC service + Observable |
| `src/adapters/service_adapters/bridge_manager.h/.cpp` | Bridge manager |
| `src/adapters/service_adapters/websocket_adapters/websocket_adapter.h/.cpp` | WebSocket transport |
| `src/adapters/service_adapters/dds_adapters/dds_adapter.h/.cpp` | DDS transport |
| `src/handlers/sensor_data_log_handler/` | Observer #1 (logging) |
| `src/handlers/sensor_data_validator/` | Observer #2 (validation) |
| `src/websocket/ws_server.h/.cpp` | WebSocket server |
| `src/dds/dds_publisher.h/.cpp` | DDS publisher |
| `proto/sensor.proto` | gRPC service definition |
| `idl/SensorData/SensorData.idl` | DDS type definition |
