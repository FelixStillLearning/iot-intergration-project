#pragma once
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include "sensor.pb.h"

/**
 * json_helper.h -- Utility untuk konversi data sensor ke format JSON
 * 
 * File ini menyediakan fungsi helper untuk mengubah SensorRequest (protobuf)
 * menjadi string JSON. JSON digunakan karena:
 *   - Universal: semua bahasa pemrograman bisa parse JSON
 *   - Human-readable: mudah dibaca saat debugging
 *   - WebSocket standard: browser/frontend mengharapkan data dalam format JSON
 * 
 * Library yang digunakan: RapidJSON
 *   - Salah satu library JSON C++ tercepat
 *   - Header-only (tidak perlu compile library terpisah)
 *   - Menggunakan DOM (Document Object Model) approach
 * 
 * Fungsi ini dipanggil oleh WebSocketAdapter::send() sebelum broadcast.
 */
namespace utils {
    /**
     * Konversi SensorRequest (protobuf) menjadi JSON string.
     * 
     * Proses:
     *   1. Buat RapidJSON Document (representasi JSON di memory)
     *   2. Tambahkan field sensor satu per satu ke document
     *   3. Serialize document ke string menggunakan RapidJSON Writer
     * 
     * Contoh output:
     *   {"sensor_id":1,"temperature":25.5,"humidity":60.0,"location":"Room A"}
     * 
     * Catatan: tidak semua field SensorRequest dikonversi ke JSON.
     * Hanya field yang diperlukan oleh frontend yang disertakan.
     * 
     * @param request Pointer ke SensorRequest (data sensor dari gRPC)
     * @return String JSON yang siap dikirim via WebSocket
     */
    inline std::string sensor_to_json(const iot::SensorRequest* request) {
        // Buat JSON document kosong bertipe Object {}
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();  // Allocator untuk memory management

        // Tambahkan field-field sensor ke JSON document
        doc.AddMember("sensor_id", request->sensor_id(), allocator);         // integer
        doc.AddMember("temperature", request->temperature(), allocator);     // double
        doc.AddMember("humidity", request->humidity(), allocator);           // double
        // String harus di-wrap dengan rapidjson::Value agar memory di-manage oleh allocator
        doc.AddMember("location", rapidjson::Value(request->location().c_str(), allocator).Move(), allocator);

        // Serialize JSON document ke string
        rapidjson::StringBuffer buffer;                         // Buffer untuk menampung output
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer); // Writer yang menulis ke buffer
        doc.Accept(writer);                                     // Traverse document dan tulis ke buffer

        return buffer.GetString();  // Kembalikan sebagai std::string
    }
}