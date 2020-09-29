#pragma once

#include <filesystem>
#include <memory>
#include <sqlite3.h>

#include "shared/aimlab/aimlab.h"

namespace service::aimlab {

class AimlabDbInterface {
public:
    AimlabDbInterface(const std::filesystem::path& path);
    ~AimlabDbInterface();

    shared::aimlab::TaskData getLatestTaskData() const;
    shared::aimlab::TaskData getTaskDataFromId(int id) const;
    std::vector<int> getAllTaskDataId() const;
private:
    std::filesystem::path _path;
    sqlite3* _db = nullptr;
};
using AimlabDbInterfacePtr = std::unique_ptr<AimlabDbInterface>;

}