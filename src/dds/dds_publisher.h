#pragma once
#include <dds/DdsDcpsPublicationC.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>
#include "SensorDataTypeSupportImpl.h"
#include <memory>
#include <string>

/**
 * DdsPublisher -- Publisher untuk OpenDDS Network
 * 
 * Class ini menangani semua interaksi dengan DDS (Data Distribution Service).
 * DDS adalah middleware komunikasi publish-subscribe yang digunakan untuk
 * mengirim data sensor ke node-node lain dalam jaringan secara real-time.
 * 
 * Komponen DDS yang dikelola:
 *   - DomainParticipant : titik masuk ke DDS domain (seperti "sesi" DDS)
 *   - Topic             : kategori/nama data yang di-publish (contoh: "TestData_Msg")
 *   - Publisher         : objek yang mengelola pengiriman data
 *   - DataWriter        : objek yang menulis data ke topic
 * 
 * Alur penggunaan:
 *   1. main() memanggil init() -> setup semua komponen DDS
 *   2. DdsAdapter memanggil publish() -> kirim data sensor ke DDS network
 *   3. Destructor -> cleanup semua resource DDS
 * 
 * Konfigurasi via environment variables:
 *   - DDS_DOMAIN : domain ID untuk DDS (default: 0)
 *   - TEST_TOPIC : nama topic DDS (default: "TestData_Msg")
 *   - DDS_CONFIG_FILE : path ke file konfigurasi RTPS (default: "rtps.ini")
 * 
 * IDL type yang digunakan: Messengger::Message (dari SensorData.idl)
 */
class DdsPublisher {
public:
    DdsPublisher();

    /**
     * Destructor: bersihkan semua resource DDS.
     * Menghapus semua entity yang dibuat (participant, topic, publisher, writer).
     */
    ~DdsPublisher();

    /**
     * Inisialisasi DDS publisher.
     * Membuat DomainParticipant, register type, buat Topic, Publisher, dan DataWriter.
     * 
     * @param argc Jumlah argumen (termasuk -DCPSConfigFile)
     * @param argv Array argumen (path ke config file DDS)
     * @return true jika semua komponen berhasil dibuat, false jika ada yang gagal
     */
    bool init(int argc, char* argv[]);

    /**
     * Publish data sensor ke DDS network.
     * Mengkonversi parameter ke format IDL (Messengger::Message) lalu mengirim via DataWriter.
     * 
     * @param id    ID sensor
     * @param name  Nama sensor
     * @param temp  Suhu (Celsius)
     * @param hum   Kelembaban (%)
     * @param press Tekanan udara (hPa)
     * @param light Intensitas cahaya (lux)
     * @param ts    Timestamp (epoch milliseconds)
     * @param loc   Lokasi sensor
     */
    void publish(long id, const std::string& name, double temp, double hum, double press, 
                 double light, long long ts, const std::string& loc);

private:
    DDS::DomainParticipant_var participant_;  // Titik masuk ke DDS domain
    DDS::Topic_var topic_;                    // Topic tempat data di-publish
    DDS::Publisher_var publisher_;            // Objek publisher DDS
    Messengger::MessageDataWriter_var writer_; // DataWriter untuk menulis ke topic
};
