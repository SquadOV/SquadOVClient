#pragma once

#include <filesystem>
#include <memory>
#include <sqlite3.h>

#include "shared/aimlab/aimlab.h"
#include "shared/time.h"

namespace service::aimlab {

class AimlabDbInterface {
public:
    AimlabDbInterface(const std::filesystem::path& path);
    ~AimlabDbInterface();

    shared::aimlab::TaskData getLatestTaskData() const;
    shared::aimlab::TaskData getTaskDataFromId(int id) const;
    std::vector<shared::aimlab::TaskData> getAllTaskDataSince(const shared::TimePoint& tm) const;
private:
    std::filesystem::path _path;
    sqlite3* _db = nullptr;
};
using AimlabDbInterfacePtr = std::unique_ptr<AimlabDbInterface>;

}