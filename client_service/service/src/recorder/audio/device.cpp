#include "recorder/audio/device.h"

namespace service::recorder::audio {

nlohmann::json AudioDeviceResponse::toJson() const {
    nlohmann::json obj;
    obj["default"] = default;
    obj["options"] = nlohmann::json::array();
    for (const auto& v : options) {
        obj["options"].push_back(v);
    }
    return obj;   
}

}