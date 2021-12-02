#include "speed_check/speed_check.h"
#include "shared/filesystem/utility.h"

namespace service::speed_check {

SpeedCheckDestination SpeedCheckDestination::fromJson(const nlohmann::json& obj) {
    SpeedCheckDestination dest;
    dest.url = obj["url"].get<std::string>();
    dest.bucket = obj["bucket"].get<std::string>();
    dest.session = obj["session"].get<std::string>();
    dest.loc = static_cast<SpeedCheckManagerType>(obj.value("loc", 0));
    return dest;
}

}

std::ostream& operator<<(std::ostream& os, const service::speed_check::SpeedCheckDestination& x) {
    os << "{url: " << x.url << ", bucket: " << x.bucket << ", session: " << x.session << ", loc: " << static_cast<int>(x.loc) << "}";
    return os;
}