/**
 * dds_adapter.cpp -- Implementasi DdsAdapter
 * 
 * File ini berisi implementasi method-method DdsAdapter.
 * DdsAdapter menerjemahkan SensorRequest (protobuf) ke format
 * yang dipahami oleh DdsPublisher (OpenDDS IDL).
 */
#include "dds_adapter.h"
#include <spdlog/spdlog.h>

/**
 * Constructor: simpan referensi ke DDS publisher.
 * Menggunakan initializer list untuk efisiensi (langsung assign, tanpa copy).
 */
DdsAdapter::DdsAdapter(std::shared_ptr<DdsPublisher> dds_publisher)
    : dds_publisher_(dds_publisher) {
}

/**
 * Inisialisasi adapter.
 * Saat ini tidak melakukan apa-apa karena DdsPublisher
 * sudah diinisialisasi terlebih dahulu di main().
 * Method ini ada untuk memenuhi kontrak ITransportAdapter.
 */
bool DdsAdapter::init() {
    spdlog::info("DDS Adapter: Initialized");
    return true;
}

/**
 * Kirim data sensor ke DDS network.
 * 
 * Proses:
 *   1. Cek apakah DDS publisher tersedia
 *   2. Ekstrak semua field dari SensorRequest (protobuf format)
 *   3. Teruskan ke DdsPublisher::publish() yang akan mengirim via OpenDDS
 * 
 * @param request Data sensor dalam format protobuf
 */
void DdsAdapter::send(const iot::SensorRequest& request) {
    if (!dds_publisher_) {
        spdlog::warn("DDS Adapter: Publisher not available");
        return;  // Tidak bisa kirim tanpa publisher
    }

    // Ekstrak field protobuf dan teruskan ke DDS publisher
    // DdsPublisher akan mengkonversi ke format IDL (Messengger::Message)
    dds_publisher_->publish(
        request.sensor_id(),         // ID sensor
        request.sensor_name(),       // Nama sensor
        request.temperature(),       // Suhu (Celsius)
        request.humidity(),          // Kelembaban (%)
        request.pressure(),          // Tekanan (hPa)
        request.light_intensity(),   // Intensitas cahaya (lux)
        request.timestamp(),         // Timestamp (epoch ms)
        request.location()           // Lokasi sensor
    );
    spdlog::debug("DDS Adapter: Sent message");
}

/**
 * Nama adapter untuk keperluan logging dan identifikasi.
 * @return String "DDS"
 */
std::string DdsAdapter::name() const {
    return "DDS";
}
