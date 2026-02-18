#include "dds_publisher.h"
#include <spdlog/spdlog.h>
#include <cstdlib>

DdsPublisher::DdsPublisher()
    : participant_(nullptr),
      topic_(nullptr),
      publisher_(nullptr),
      writer_(nullptr) {
}

DdsPublisher::~DdsPublisher() {
    if (!CORBA::is_nil(participant_.in())) {
        participant_->delete_contained_entities();
        DDS::DomainParticipantFactory_var dpf = TheParticipantFactory;
        if (!CORBA::is_nil(dpf.in())) {
            dpf->delete_participant(participant_.in());
        }
    }
}

bool DdsPublisher::init(int argc, char* argv[]) {
    try {
        DDS::DomainParticipantFactory_var dpf = TheParticipantFactoryWithArgs(argc, argv);
        
        if (CORBA::is_nil(dpf.in())) {
            spdlog::error("DDS: DomainParticipantFactory is nil");
            return false;
        }

        // Get domain from env or default to 0
        int domain = 0;
        const char* domain_env = std::getenv("DDS_DOMAIN");
        if (domain_env && *domain_env) {
            try { domain = std::stoi(domain_env); } catch (...) {}
        }

        participant_ = dpf->create_participant(
            domain,
            PARTICIPANT_QOS_DEFAULT,
            DDS::DomainParticipantListener::_nil(),
            OpenDDS::DCPS::DEFAULT_STATUS_MASK
        );

        if (CORBA::is_nil(participant_.in())) {
            spdlog::error("DDS: Failed to create DomainParticipant");
            return false;
        }

        // Register the IDL type
        Messengger::MessageTypeSupport_var ts = new Messengger::MessageTypeSupportImpl();
        if (ts->register_type(participant_.in(), "") != DDS::RETCODE_OK) {
            spdlog::error("DDS: Failed to register type");
            return false;
        }

        // Get topic name from env or default
        std::string topic_name = "TestData_Msg";
        const char* topic_env = std::getenv("TEST_TOPIC");
        if (topic_env && *topic_env) {
            topic_name = topic_env;
        }

        CORBA::String_var type_name = ts->get_type_name();
        topic_ = participant_->create_topic(
            topic_name.c_str(),
            type_name.in(),
            TOPIC_QOS_DEFAULT,
            DDS::TopicListener::_nil(),
            OpenDDS::DCPS::DEFAULT_STATUS_MASK
        );

        if (CORBA::is_nil(topic_.in())) {
            spdlog::error("DDS: Failed to create Topic '{}'", topic_name);
            return false;
        }

        // Create publisher
        publisher_ = participant_->create_publisher(
            PUBLISHER_QOS_DEFAULT,
            DDS::PublisherListener::_nil(),
            OpenDDS::DCPS::DEFAULT_STATUS_MASK
        );

        if (CORBA::is_nil(publisher_.in())) {
            spdlog::error("DDS: Failed to create Publisher");
            return false;
        }

        // Create DataWriter
        DDS::DataWriter_var dw = publisher_->create_datawriter(
            topic_.in(),
            DATAWRITER_QOS_DEFAULT,
            DDS::DataWriterListener::_nil(),
            OpenDDS::DCPS::DEFAULT_STATUS_MASK
        );

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

void DdsPublisher::publish(long id, const std::string& name, double temp, double hum, 
                           double press, double light, long long ts, const std::string& loc) {
    if (CORBA::is_nil(writer_.in())) {
        spdlog::warn("DDS: DataWriter not initialized");
        return;
    }

    Messengger::Message msg;
    msg.sensor_id = static_cast<CORBA::Long>(id);
    msg.sensor_name = name.c_str();
    msg.temperature = temp;
    msg.humidity = hum;
    msg.pressure = press;
    msg.light_intensity = light;
    msg.timestamp = static_cast<CORBA::LongLong>(ts);
    msg.location = loc.c_str();

    DDS::ReturnCode_t ret = writer_->write(msg, DDS::HANDLE_NIL);
    if (ret == DDS::RETCODE_OK) {
        spdlog::info("DDS: Published - ID: {}, Name: {}, Temp: {}C", id, name, temp);
    } else {
        spdlog::error("DDS: Write failed with code {}", static_cast<int>(ret));
    }
}
