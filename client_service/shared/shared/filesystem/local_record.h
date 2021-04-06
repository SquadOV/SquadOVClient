#pragma once
#include "shared/json.h"
#include "shared/time.h"
#include "shared/http/http_client.h"
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <sqlite3.h>
#include <vector>

namespace shared::filesystem {

struct ChangeLocalRecordRequest {
    std::string from;
    std::string to;
};

struct CleanupLocalRecordRequest {
    std::string loc;
    double limit;
};

struct LocalRecordingIndexEntry {
    std::string uuid;
    std::string filename;
    shared::TimePoint startTm;
    shared::TimePoint endTm;
    shared::TimePoint cacheTm;
    size_t diskBytes;
};

class LocalRecordingIndexDb {
public:
    static LocalRecordingIndexDb* singleton();
    ~LocalRecordingIndexDb();

    void initializeFromFolder(const std::filesystem::path& parentFolder);
    bool moveLocalFolderTo(const std::filesystem::path& to);
    bool cleanupLocalFolder(double limit);

    void addLocalEntryFromUri(const std::string& uri, const LocalRecordingIndexEntry& entry, const shared::http::DownloadProgressFn& progressFn);
    void addLocalEntryFromFilesystem(const std::filesystem::path& file, const LocalRecordingIndexEntry& entry);
    void removeLocalEntry(const std::string& uuid);

    std::vector<LocalRecordingIndexEntry> getAllLocalEntries() const;
    std::optional<LocalRecordingIndexEntry> getOldestLocalEntry() const;
    std::optional<LocalRecordingIndexEntry> getEntryForUuid(const std::string& uuid) const;

    std::filesystem::path getEntryPath(const LocalRecordingIndexEntry& entry) const;
    std::filesystem::path getEntryPath(const std::filesystem::path& parent, const LocalRecordingIndexEntry& entry) const;

private:
    void migrateLocalEntry(const LocalRecordingIndexEntry& entry, const std::filesystem::path& to) const;
    void cleanupLocalEntry(const LocalRecordingIndexEntry& entry) const;
    void migrateDatabase() const;

    mutable std::recursive_mutex _dbMutex;
    std::optional<std::filesystem::path> _initFolder;
    sqlite3* _db = nullptr;

    void release();

};
using LocalRecordingIndexDbPtr = std::unique_ptr<LocalRecordingIndexDb>;

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