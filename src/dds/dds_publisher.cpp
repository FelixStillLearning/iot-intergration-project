#include "dds_publisher.h"
#include <spdlog/spdlog.h>

DdsPublisher::DdsPublisher()
    : participant_(nullptr),
      topic_(nullptr),
      publisher_(nullptr) {
}

DdsPublisher::~DdsPublisher() {
    if (!CORBA::is_nil(participant_.in())) {
        participant_->delete_contained_entities();
        DDS::DomainParticipantFactory_var dpf = DDS::TheDomainParticipantFactory;
        dpf->delete_participant(participant_.in());
    }
}

bool DdsPublisher::init(int argc, char* argv[]) {
    try {
        DDS::DomainParticipantFactory_var dpf = DDS::TheDomainParticipantFactory;
        
        if (CORBA::is_nil(dpf.in())) {
            spdlog::error("DDS: DomainParticipantFactory is nil");
            return false;
        }

        participant_ = dpf->create_participant(
            0,
            PARTICIPANT_QOS_DEFAULT,
            DDS::DomainParticipantListener::_nil(),
            OpenDDS::DCPS::DEFAULT_STATUS_MASK
        );

        if (CORBA::is_nil(participant_.in())) {
            spdlog::error("DDS: Failed to create DomainParticipant");
            return false;
        }

        DDS::PublisherQos publisher_qos;
        participant_->get_default_publisher_qos(publisher_qos);

        publisher_ = participant_->create_publisher(
            publisher_qos,
            DDS::PublisherListener::_nil(),
            OpenDDS::DCPS::DEFAULT_STATUS_MASK
        );

        if (CORBA::is_nil(publisher_.in())) {
            spdlog::error("DDS: Failed to create Publisher");
            return false;
        }

        spdlog::info("DDS: Initialized successfully");
        return true;

    } catch (const std::exception& e) {
        spdlog::error("DDS: Initialization failed - {}", e.what());
        return false;
    }
}

void DdsPublisher::publish(long id, const std::string& name, double temp, double hum, 
                           double press, double light, long long ts, const std::string& loc) {
    if (CORBA::is_nil(publisher_.in())) {
        spdlog::warn("DDS: Publisher not initialized");
        return;
    }

    spdlog::info("DDS: Publishing - ID: {}, Name: {}, Temp: {}C", id, name, temp);
}
