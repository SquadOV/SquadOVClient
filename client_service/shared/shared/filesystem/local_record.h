#pragma once
#include "shared/json.h"
#include "shared/time.h"
#include "shared/http/http_client.h"
#include <filesystem>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <optional>
#include <unordered_map>
#include <thread>
#include <vector>

#include <Windows.h>

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
    std::filesystem::path root;
    std::filesystem::path relative;
    shared::TimePoint lastWriteTime;
    size_t diskBytes;

    std::filesystem::path fullPath() const;
};

class LocalRecordingIndexDb {
public:
    static LocalRecordingIndexDb* singleton();
    LocalRecordingIndexDb();
    ~LocalRecordingIndexDb();

    void initializeFromFolder(const std::filesystem::path& parentFolder);
    bool moveLocalFolderTo(const std::filesystem::path& to);
    bool cleanupLocalFolder(double limit);

    void addLocalEntryFromUri(const std::string& uri, const std::string& md5Checksum, const LocalRecordingIndexEntry& entry, const shared::http::DownloadUploadProgressFn& progressFn);
    void addLocalEntryFromFilesystem(const std::filesystem::path& file, const LocalRecordingIndexEntry& entry);
    void removeVideoFromDatabase(const std::string& uuid);
    size_t currentSizeBytes() const;

    std::vector<LocalRecordingIndexEntry> getAllLocalEntries() const;
    std::optional<LocalRecordingIndexEntry> getOldestLocalEntry() const;
    std::optional<LocalRecordingIndexEntry> getEntryForUuid(const std::string& uuid) const;

private:
    void migrateToV2() const;
    void rebuildDatabase();
    bool addMetadataToFile(const std::filesystem::path& path, const std::string& videoUuid) const;
    std::optional<LocalRecordingIndexEntry> buildEntryFromFile(const std::filesystem::path& path) const;

    std::optional<std::filesystem::path> _initFolder;

    mutable std::shared_mutex _dbMutex;
    std::unordered_map<std::string, LocalRecordingIndexEntry> _uuidDatabase;
    std::unordered_map<std::string, LocalRecordingIndexEntry> _pathDatabase;

    void removeFileFromDatabase(const std::filesystem::path& path);
    void removeEntryFromDatabase(const LocalRecordingIndexEntry& entry);
    void addEntryToDatabase(const LocalRecordingIndexEntry& entry);

    // Incremental database updates
    std::thread _watchThread;
    bool _watchRunning = false;
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