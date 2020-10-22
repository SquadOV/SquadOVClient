#pragma once

#include "shared/time.h"
#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>

namespace service::local {

// Stores local simple key-value data per-user in a JSON file on disk.
class LocalData {
public:
    LocalData();

    // Aim Lab
    shared::TimePoint getLastAimlabBackfillTime() const;
    void markAimlabBackfillTime(const shared::TimePoint& tm);
private:
    void flush();

    std::filesystem::path _path;
    nlohmann::json _data;
};
using LocalDataPtr = std::unique_ptr<LocalData>;

LocalData* getLocalData();

}