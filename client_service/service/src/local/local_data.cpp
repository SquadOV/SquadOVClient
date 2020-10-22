#include "local/local_data.h"

#include "shared/filesystem/common_paths.h"

#include <fstream>

namespace service::local {
namespace {

const std::string AIMLAB_BACKFILL_TIME_KEY = "aimlab_backfill_time";

}

LocalData* getLocalData() {
    static LocalDataPtr ptr = std::make_unique<LocalData>();
    return ptr.get();
}

LocalData::LocalData() {
    _path = shared::filesystem::getSquadOvUserFolder() / std::filesystem::path("local_data.json");
    if (std::filesystem::exists(_path)) {
        std::ifstream input(_path.string());
        _data = nlohmann::json::parse(input);
    } else {
        _data = nlohmann::json::object();
    }
}

shared::TimePoint LocalData::getLastAimlabBackfillTime() const {
    if (_data.find(AIMLAB_BACKFILL_TIME_KEY) == _data.end()) {
        // Use a custom min here instead of max because we effectively want to say we *never* have done a backfill.
        // We want to use a custom min here because SQLite is a pain in the butt and can only do string comparison.
        // If we use the min() of TimePoint then we get a negative year and we'll get wonky comparisons...
        return shared::zeroTime();
    }

    return shared::strToTime(_data[AIMLAB_BACKFILL_TIME_KEY]);
}

void LocalData::markAimlabBackfillTime(const shared::TimePoint& tm) {
    if (tm < getLastAimlabBackfillTime()) {
        return;
    }
    _data[AIMLAB_BACKFILL_TIME_KEY] = shared::timeToStr(tm);
    flush();
}

void LocalData::flush() {
    std::ofstream output(_path.string(), std::ios_base::trunc);
    output << _data;
}

}