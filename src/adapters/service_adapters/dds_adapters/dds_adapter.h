#pragma once
#include "adapters/interface_adapters/interface_transport_adapter.h"
#include "dds/dds_publisher.h"
#include <memory>

/**
 * DdsAdapter -- Concrete Transport Adapter untuk OpenDDS
 * 
 * Adapter ini menjembatani SensorController ke DDS network.
 * Ketika data sensor masuk via gRPC, BridgeManager memanggil send()
 * pada DdsAdapter, yang kemudian meneruskan data ke DdsPublisher.
 * 
 * Alur data:
 *   BridgeManager::broadcast_sensor_data()
 *       -> DdsAdapter::send(request)
 *           -> DdsPublisher::publish(id, name, temp, ...)
 *               -> OpenDDS network (RTPS protocol)
 * 
 * DDS (Data Distribution Service) digunakan untuk:
 *   - Komunikasi real-time antar node/mesin
 *   - Distribusi data sensor skala besar
 *   - Komunikasi yang reliable dan low-latency
 * 
 * Class ini TIDAK memiliki logic DDS secara langsung.
 * Semua logic DDS ada di DdsPublisher. DdsAdapter hanya berperan
 * sebagai "adapter" yang menerjemahkan SensorRequest ke format DDS.
 */
class DdsAdapter : public ITransportAdapter {
public:
    /**
     * Constructor: terima shared pointer ke DdsPublisher.
     * DdsPublisher sudah diinisialisasi di main() sebelum adapter dibuat.
     * @param dds_publisher Instance DDS publisher yang sudah terinisialisasi
     */
    explicit DdsAdapter(std::shared_ptr<DdsPublisher> dds_publisher);
    
    /**
     * Inisialisasi adapter. Saat ini hanya log bahwa adapter siap.
     * @return true selalu (DDS publisher sudah diinit di main())
     */
    bool init() override;

    /**
     * Kirim data sensor ke DDS network.
     * Mengekstrak field dari SensorRequest dan meneruskan ke DdsPublisher::publish().
     * @param request Data sensor yang akan dikirim via DDS
     */
    void send(const iot::SensorRequest& request) override;

    /**
     * Nama adapter untuk logging.
     * @return "DDS"
     */
    std::string name() const override;

private:
    std::shared_ptr<DdsPublisher> dds_publisher_;  // Referensi ke DDS publisher
};
