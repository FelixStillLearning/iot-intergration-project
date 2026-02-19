/**
 * dds_publisher.cpp -- Implementasi DdsPublisher
 * 
 * File ini berisi implementasi lengkap untuk inisialisasi dan publishing
 * data melalui OpenDDS. OpenDDS menggunakan protokol RTPS (Real-Time
 * Publish-Subscribe) untuk komunikasi antar-node secara peer-to-peer.
 * 
 * Urutan inisialisasi DDS (wajib berurutan):
 *   1. Dapatkan DomainParticipantFactory (singleton, entry point DDS)
 *   2. Buat DomainParticipant (bergabung ke domain tertentu)
 *   3. Register type (mapping antara IDL type dan DDS)
 *   4. Buat Topic (kategori data yang akan di-publish)
 *   5. Buat Publisher (pengelola pengiriman)
 *   6. Buat DataWriter (penulis data ke topic)
 * 
 * Jika salah satu langkah gagal, init() mengembalikan false
 * dan aplikasi akan berhenti (lihat main() di app.cpp).
 */
#include "dds_publisher.h"
#include <spdlog/spdlog.h>
#include <cstdlib>

/**
 * Constructor: inisialisasi semua member ke null.
 * CORBA menggunakan _var (smart pointer) yang auto-release saat destructor.
 */
DdsPublisher::DdsPublisher()
    : participant_(nullptr),
      topic_(nullptr),
      publisher_(nullptr),
      writer_(nullptr) {
}

/**
 * Destructor: bersihkan semua entity DDS secara berurutan.
 * 
 * Penting: DDS mengharuskan cleanup dilakukan dalam urutan terbalik
 * dari pembuatan. delete_contained_entities() menghapus semua child
 * entity (topic, publisher, writer) sekaligus.
 */
DdsPublisher::~DdsPublisher() {
    if (!CORBA::is_nil(participant_.in())) {
        participant_->delete_contained_entities();  // Hapus semua child entity
        DDS::DomainParticipantFactory_var dpf = TheParticipantFactory;
        if (!CORBA::is_nil(dpf.in())) {
            dpf->delete_participant(participant_.in());  // Hapus participant dari factory
        }
    }
}

/**
 * Inisialisasi lengkap DDS publisher.
 * 
 * Langkah-langkah:
 *   1. Dapatkan DomainParticipantFactory dengan argumen config file
 *   2. Buat DomainParticipant di domain tertentu (dari env DDS_DOMAIN)
 *   3. Register IDL type (Messengger::Message) ke participant
 *   4. Buat Topic dengan nama dari env TEST_TOPIC
 *   5. Buat Publisher dengan QoS default
 *   6. Buat DataWriter dan narrow ke type-specific writer
 * 
 * @param argc Jumlah argumen (termasuk -DCPSConfigFile)
 * @param argv Array argumen command line
 * @return true jika semua berhasil, false jika ada kegagalan
 */
bool DdsPublisher::init(int argc, char* argv[]) {
    try {
        // Langkah 1: Dapatkan factory dengan argumen config
        // TheParticipantFactoryWithArgs adalah macro OpenDDS yang
        // mem-parse argumen dan mengembalikan singleton factory
        DDS::DomainParticipantFactory_var dpf = TheParticipantFactoryWithArgs(argc, argv);
        
        if (CORBA::is_nil(dpf.in())) {
            spdlog::error("DDS: DomainParticipantFactory is nil");
            return false;
        }

        // Langkah 2: Baca domain ID dari environment variable
        // Domain ID menentukan "ruang" komunikasi DDS (node di domain berbeda tidak bisa berkomunikasi)
        int domain = 0;
        const char* domain_env = std::getenv("DDS_DOMAIN");
        if (domain_env && *domain_env) {
            try { domain = std::stoi(domain_env); } catch (...) {}
        }

        // Buat DomainParticipant di domain yang ditentukan
        participant_ = dpf->create_participant(
            domain,                                    // Domain ID
            PARTICIPANT_QOS_DEFAULT,                    // QoS default
            DDS::DomainParticipantListener::_nil(),     // Tidak ada listener
            OpenDDS::DCPS::DEFAULT_STATUS_MASK          // Status mask default
        );

        if (CORBA::is_nil(participant_.in())) {
            spdlog::error("DDS: Failed to create DomainParticipant");
            return false;
        }

        // Langkah 3: Register IDL type ke DDS
        // TypeSupport memberi tahu DDS cara serialisasi/deserialisasi Messengger::Message
        Messengger::MessageTypeSupport_var ts = new Messengger::MessageTypeSupportImpl();
        if (ts->register_type(participant_.in(), "") != DDS::RETCODE_OK) {
            spdlog::error("DDS: Failed to register type");
            return false;
        }

        // Langkah 4: Buat Topic
        // Topic name bisa di-override via env TEST_TOPIC
        std::string topic_name = "TestData_Msg";
        const char* topic_env = std::getenv("TEST_TOPIC");
        if (topic_env && *topic_env) {
            topic_name = topic_env;
        }

        CORBA::String_var type_name = ts->get_type_name();  // Ambil nama type yang diregister
        topic_ = participant_->create_topic(
            topic_name.c_str(),                        // Nama topic
            type_name.in(),                            // Nama type IDL
            TOPIC_QOS_DEFAULT,                         // QoS default
            DDS::TopicListener::_nil(),                // Tidak ada listener
            OpenDDS::DCPS::DEFAULT_STATUS_MASK         // Status mask default
        );

        if (CORBA::is_nil(topic_.in())) {
            spdlog::error("DDS: Failed to create Topic '{}'", topic_name);
            return false;
        }

        // Langkah 5: Buat Publisher
        // Publisher mengelola DataWriter dan mengatur pengiriman data
        publisher_ = participant_->create_publisher(
            PUBLISHER_QOS_DEFAULT,                     // QoS default
            DDS::PublisherListener::_nil(),             // Tidak ada listener
            OpenDDS::DCPS::DEFAULT_STATUS_MASK         // Status mask default
        );

        if (CORBA::is_nil(publisher_.in())) {
            spdlog::error("DDS: Failed to create Publisher");
            return false;
        }

        // Langkah 6: Buat DataWriter dan narrow ke type-specific writer
        // DataWriter generik di-narrow ke Messengger::MessageDataWriter
        // agar bisa menulis Messengger::Message secara type-safe
        DDS::DataWriter_var dw = publisher_->create_datawriter(
            topic_.in(),                               // Topic yang dituju
            DATAWRITER_QOS_DEFAULT,                    // QoS default
            DDS::DataWriterListener::_nil(),            // Tidak ada listener
            OpenDDS::DCPS::DEFAULT_STATUS_MASK         // Status mask default
        );

        // Narrow: konversi dari DataWriter generik ke type-specific writer
        writer_ = Messengger::MessageDataWriter::_narrow(dw.in());
        if (CORBA::is_nil(writer_.in())) {
            spdlog::error("DDS: Failed to narrow DataWriter");
            return false;
        }

        spdlog::info("DDS: Initialized successfully (domain={}, topic={})", domain, topic_name);
        return true;

    } catch (const CORBA::Exception& e) {
        spdlog::error("DDS: CORBA exception during init");
        return false;
    } catch (const std::exception& e) {
        spdlog::error("DDS: Initialization failed - {}", e.what());
        return false;
    }
}

/**
 * Publish data sensor ke DDS network.
 * 
 * Proses:
 *   1. Cek apakah DataWriter sudah terinisialisasi
 *   2. Buat Messengger::Message (IDL struct) dan isi field-fieldnya
 *   3. Tulis message ke topic via DataWriter
 *   4. DDS akan otomatis mengirim ke semua subscriber di domain yang sama
 * 
 * @param id    ID sensor
 * @param name  Nama sensor
 * @param temp  Suhu dalam Celsius
 * @param hum   Kelembaban dalam persen
 * @param press Tekanan udara dalam hPa
 * @param light Intensitas cahaya dalam lux
 * @param ts    Timestamp dalam epoch milliseconds
 * @param loc   Lokasi sensor (string deskriptif)
 */
void DdsPublisher::publish(long id, const std::string& name, double temp, double hum, 
                           double press, double light, long long ts, const std::string& loc) {
    if (CORBA::is_nil(writer_.in())) {
        spdlog::warn("DDS: DataWriter not initialized");
        return;
    }

    // Buat IDL message dan isi semua field
    // Messengger::Message didefinisikan di SensorData.idl
    Messengger::Message msg;
    msg.sensor_id = static_cast<CORBA::Long>(id);           // Cast ke CORBA type
    msg.sensor_name = name.c_str();                         // String ke CORBA string
    msg.temperature = temp;
    msg.humidity = hum;
    msg.pressure = press;
    msg.light_intensity = light;
    msg.timestamp = static_cast<CORBA::LongLong>(ts);       // Cast ke CORBA 64-bit
    msg.location = loc.c_str();

    // Tulis ke DDS topic
    // HANDLE_NIL berarti DDS akan auto-register instance
    DDS::ReturnCode_t ret = writer_->write(msg, DDS::HANDLE_NIL);
    if (ret == DDS::RETCODE_OK) {
        spdlog::info("DDS: Published - ID: {}, Name: {}, Temp: {}C", id, name, temp);
    } else {
        spdlog::error("DDS: Write failed with code {}", static_cast<int>(ret));
    }
}
