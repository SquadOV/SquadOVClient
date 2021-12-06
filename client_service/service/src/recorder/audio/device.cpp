#include "recorder/audio/device.h"

namespace service::recorder::audio {

nlohmann::json SingleAudioDevice::toJson() const {
    nlohmann::json obj;
    obj["name"] = name;
    obj["id"] = id;
    return obj;
}

nlohmann::json AudioDeviceResponse::toJson() const {
    nlohmann::json obj;
    obj["default"] = default.toJson();
    obj["options"] = nlohmann::json::array();
    for (const auto& v : options) {
        obj["options"].push_back(v.toJson());
    }
    return obj;   
}

}