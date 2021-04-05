#include "shared/filesystem/local_record.h"

namespace shared::filesystem {

bool changeLocalRecordLocation(const std::filesystem::path& from, const std::filesystem::path& to) {
    return true;
}

bool cleanupLocalRecordLocation(const std::filesystem::path& location, double limit) {
    return true;
}

}