#pragma once
#include "shared/json.h"
#include <filesystem>

namespace shared::filesystem {

struct ChangeLocalRecordRequest {
    std::string from;
    std::string to;
};

struct CleanupLocalRecordRequest {
    std::string loc;
    double limit;
};

bool changeLocalRecordLocation(const std::filesystem::path& from, const std::filesystem::path& to);
bool cleanupLocalRecordLocation(const std::filesystem::path& location, double limit);

}

namespace shared::json {

template<>
struct JsonConverter<shared::filesystem::ChangeLocalRecordRequest> {
    static shared::filesystem::ChangeLocalRecordRequest from(const nlohmann::json& v) {
        shared::filesystem::ChangeLocalRecordRequest val;
        val.from = v["from"].get<std::string>();
        val.to = v["to"].get<std::string>();
        return val;
    }
};
    
template<>
struct JsonConverter<shared::filesystem::CleanupLocalRecordRequest> {
    static shared::filesystem::CleanupLocalRecordRequest from(const nlohmann::json& v) {
        shared::filesystem::CleanupLocalRecordRequest val;
        val.loc = v["loc"].get<std::string>();
        val.limit = v["limit"].get<double>();
        return val;
    }
};

}