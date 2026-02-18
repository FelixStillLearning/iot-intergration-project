#pragma once
#include <dds/DdsDcpsPublicationC.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>
#include <memory>

class DdsPublisher {
public:
    DdsPublisher();
    ~DdsPublisher();

    bool init(int argc, char* argv[]);
    void publish(long id, const std::string& name, double temp, double hum, double press, 
                 double light, long long ts, const std::string& loc);

private:
    DDS::DomainParticipant_var participant_;
    DDS::Topic_var topic_;
    DDS::Publisher_var publisher_;
};
