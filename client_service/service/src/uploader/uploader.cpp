#include "uploader/uploader.h"
#include "shared/filesystem/utility.h"

namespace service::uploader {

UploadDestination UploadDestination::local(const std::filesystem::path& path) {
    UploadDestination dest;
    dest.url = shared::filesystem::pathUtf8(path); 
    dest.loc = UploadManagerType::FileSystem;
    return dest;
}

UploadDestination UploadDestination::fromJson(const nlohmann::json& obj) {
    UploadDestination dest;
    dest.url = obj["url"].get<std::string>();
    dest.bucket = obj["bucket"].get<std::string>();
    dest.session = obj["session"].get<std::string>();
    dest.loc = static_cast<UploadManagerType>(obj.value("loc", 0));
    return dest;
}

std::ostream& operator<<(std::ostream& os, const service::uploader::UploadDestination& x) {
    os << "{url: " << x.url << ", bucket: " << x.bucket << ", session: " << x.session << ", loc: " << static_cast<int>(x.loc) << "}";
    return os;
}

}