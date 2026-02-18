#pragma once
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include "sensor.pb.h"

namespace utils {
    inline std::string sensor_to_json(const iot::SensorRequest* request) {
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        doc.AddMember("sensor_id", request->sensor_id(), allocator);
        doc.AddMember("temperature", request->temperature(), allocator);
        doc.AddMember("humidity", request->humidity(), allocator);
        doc.AddMember("location", rapidjson::Value(request->location().c_str(), allocator).Move(), allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        return buffer.GetString();
    }
}