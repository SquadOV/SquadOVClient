#include "vod/vod.h"
#include "shared/filesystem/utility.h"

namespace service::vod {

VodDestination VodDestination::local(const std::filesystem::path& path) {
    VodDestination dest;
    dest.url = shared::filesystem::pathUtf8(path); 
    dest.loc = VodManagerType::FileSystem;
    return dest;
}

VodDestination VodDestination::fromJson(const nlohmann::json& obj) {
    VodDestination dest;
    dest.url = obj["url"].get<std::string>();
    dest.bucket = obj["bucket"].get<std::string>();
    dest.session = obj["session"].get<std::string>();
    dest.loc = static_cast<VodManagerType>(obj.value("loc", 0));
    return dest;
}

}

std::ostream& operator<<(std::ostream& os, const service::vod::VodDestination& x) {
    os << "{url: " << x.url << ", bucket: " << x.bucket << ", session: " << x.session << ", loc: " << static_cast<int>(x.loc) << "}";
    return os;
}