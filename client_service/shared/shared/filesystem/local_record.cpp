#include "shared/filesystem/local_record.h"
#include "shared/filesystem/utility.h"
#include "shared/filesystem/common_paths.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"
#include "shared/sqlite/sqlite.h"
#include "shared/time.h"
#include "shared/http/http_client.h"

namespace fs = std::filesystem;
namespace shared::filesystem {
namespace {

constexpr int CURRENT_DB_VERSION = 1;

LocalRecordingIndexEntry getLocalEntryFromSqlStatement(shared::sqlite::SqlStatement& stmt) {
    LocalRecordingIndexEntry data;
    data.uuid = stmt.getColumn<std::string>(0);
    data.filename = stmt.getColumn<std::string>(1);
    data.startTm = shared::unixMsToTime(stmt.getColumn<long long>(2));
    data.endTm = shared::unixMsToTime(stmt.getColumn<long long>(3));
    data.cacheTm = shared::unixMsToTime(stmt.getColumn<long long>(4));
    data.diskBytes = static_cast<size_t>(stmt.getColumn<long long>(5));
    return data;
}

}

LocalRecordingIndexDb* LocalRecordingIndexDb::singleton() {
    static LocalRecordingIndexDbPtr s = std::make_unique<LocalRecordingIndexDb>();
    return s.get();
}

LocalRecordingIndexDb::~LocalRecordingIndexDb() {
    release();
}

void LocalRecordingIndexDb::initializeFromFolder(const fs::path& parentFolder) {
    std::lock_guard<std::recursive_mutex> guard(_dbMutex);
    if (_initFolder.has_value() && _initFolder.value() == parentFolder) {
        return;
    }

    release();

    if (!fs::exists(parentFolder)) {
        fs::create_directories(parentFolder);   
    }

    const auto indexFile = parentFolder / fs::path("index.db");
    LOG_INFO("Initializing local recording database: " << indexFile << std::endl);
    if (sqlite3_open(shared::filesystem::pathUtf8(indexFile).c_str(), &_db) != SQLITE_OK) {
        THROW_ERROR("Failed to open database.");
    }
    migrateDatabase();

    _initFolder = parentFolder;
}

void LocalRecordingIndexDb::migrateDatabase() const {
    std::lock_guard<std::recursive_mutex> guard(_dbMutex);
    int existing_version = 0;
    {
        std::ostringstream sql;
        sql << R"|(
            PRAGMA user_version
        )|";

        shared::sqlite::SqlStatement stmt(_db, sql.str());
        stmt.next();
        existing_version = stmt.getColumn<int>(0);
    }
    LOG_INFO("Local Record Index Db Version: " << existing_version << std::endl);

    if (existing_version < 1) {
        std::ostringstream sql;
        sql << R"|(
            CREATE TABLE local_vod_entries (
                uuid TEXT PRIMARY KEY,
                filename TEXT NOT NULL,
                start_time INTEGER NOT NULL,
                end_time INTEGER NOT NULL,
                cache_time INTEGER NOT NULL,
                disk_bytes INTEGER NOT NULL
            );
        )|";
        shared::sqlite::SqlStatement stmt(_db, sql.str());
        stmt.next();
    }

    if (existing_version != CURRENT_DB_VERSION) {
        std::ostringstream sql;
        sql << "PRAGMA user_version = " << CURRENT_DB_VERSION;
        shared::sqlite::SqlStatement stmt(_db, sql.str());
        stmt.next();
    }

    LOG_INFO("Successful migration to: " << CURRENT_DB_VERSION << std::endl);
}

bool LocalRecordingIndexDb::moveLocalFolderTo(const fs::path& to) {
    if (!_initFolder.has_value()) {
        LOG_WARNING("Attempting to migrate local recording folder when index has not been initialized." << std::endl);
        return true;
    }

    try {
        const auto oldFolder = _initFolder.value();
        const auto entries = getAllLocalEntries();
        for (const auto& e : entries) {
            migrateLocalEntry(e, to);
        }

        release();

        fs::rename(_initFolder.value() / fs::path("index.db"), to / fs::path("index.db"));
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to migrate local entry: " << ex.what() << std::endl);
        return false;
    }

    initializeFromFolder(to);
    return true;
}

void LocalRecordingIndexDb::migrateLocalEntry(const LocalRecordingIndexEntry& entry, const std::filesystem::path& to) const {
    const auto source = getEntryPath(entry);
    const auto dest = getEntryPath(to, entry);
    fs::create_directories(dest.parent_path());
    fs::rename(source, dest);
}

bool LocalRecordingIndexDb::cleanupLocalFolder(double limit) {
    if (!_initFolder.has_value()) {
        LOG_WARNING("Attempting to cleanup local recording folder when index has not been initialized." << std::endl);
        return true;
    }

    try {
        double currentSize = getFolderSizeBytes(_initFolder.value()) / 1024.0 / 1024.0 / 1024.0;
        while (currentSize > limit) {
            const auto entry = getOldestLocalEntry();
            if (!entry.has_value()) {
                break;
            }

            const auto val = entry.value();
            cleanupLocalEntry(val);
            currentSize -= val.diskBytes / 1024.0 / 1024.0 / 1024.0;
        }

        return true;
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to cleanup local entries: " << ex.what() << std::endl);
        return false;
    }
}

void LocalRecordingIndexDb::removeLocalEntry(const std::string& uuid) {
    auto entry = getEntryForUuid(uuid);
    if (!entry.has_value()) {
        return;
    }
    cleanupLocalEntry(entry.value());
}

void LocalRecordingIndexDb::cleanupLocalEntry(const LocalRecordingIndexEntry& entry) const {
    const auto folder = getEntryPath(entry).parent_path();
    if (fs::exists(folder)) {
        fs::remove_all(folder);
    }

    std::ostringstream sql;
    sql << R"|(
        DELETE FROM local_vod_entries
        WHERE uuid = ?)|";

    shared::sqlite::SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, entry.uuid);
    stmt.next();

    if (stmt.fail()) {
        THROW_ERROR("Failed to delete local entry from index DB: " << stmt.errMsg());
    }
}

void LocalRecordingIndexDb::addLocalEntryFromUri(const std::string& uri, const LocalRecordingIndexEntry& entry, const shared::http::DownloadProgressFn& progressFn) {
    const auto dlPath = shared::filesystem::getSquadOvTempFolder()  / fs::path(entry.uuid) / fs::path(entry.filename);
    fs::create_directories(dlPath.parent_path());

    shared::http::HttpClient client(uri);
    client.addDownloadProgressFn(progressFn);
    const auto resp = client.download("", dlPath);
    if (resp->status != 200) {
        THROW_ERROR("Failed to download URI: " << resp->body << std::endl);
    }

    auto sizedEntry = entry;
    sizedEntry.diskBytes = fs::file_size(dlPath);
    addLocalEntryFromFilesystem(dlPath, sizedEntry);
}

void LocalRecordingIndexDb::addLocalEntryFromFilesystem(const std::filesystem::path& file, const LocalRecordingIndexEntry& entry) {
    std::ostringstream sql;
    sql << R"|(
        INSERT INTO local_vod_entries (
            uuid,
            filename,
            start_time,
            end_time,
            cache_time,
            disk_bytes
        )
        VALUES (
            ?, ?, ?, ?, ?, ?
        )
        ON CONFLICT DO NOTHING
    )|";

    shared::sqlite::SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, entry.uuid);
    stmt.bindParameter(2, entry.filename);
    stmt.bindParameter(3, shared::timeToUnixMs(entry.startTm));
    stmt.bindParameter(4, shared::timeToUnixMs(entry.endTm));
    stmt.bindParameter(5, shared::timeToUnixMs(entry.cacheTm));
    stmt.bindParameter(6, static_cast<int64_t>(entry.diskBytes));
    stmt.next();

    if (stmt.fail()) {
        THROW_ERROR("Failed to add local entry: " << stmt.errMsg() << " [" << file << "]");
    }

    const auto outputPath = getEntryPath(entry);
    fs::create_directories(outputPath.parent_path());
    fs::rename(file, outputPath);
}

std::filesystem::path LocalRecordingIndexDb::getEntryPath(const std::filesystem::path& parent, const LocalRecordingIndexEntry& entry) const {
    return parent / fs::path(entry.uuid) / fs::path(entry.filename);
}

std::filesystem::path LocalRecordingIndexDb::getEntryPath(const LocalRecordingIndexEntry& entry) const {
    if (!_initFolder.has_value()) {
        return {};
    }

    return getEntryPath(_initFolder.value(), entry);
}

std::optional<LocalRecordingIndexEntry> LocalRecordingIndexDb::getOldestLocalEntry() const {
    std::lock_guard<std::recursive_mutex> guard(_dbMutex);

    std::ostringstream sql;
    sql << R"|(
        SELECT *
        FROM local_vod_entries
        ORDER BY cache_time DESC
        LIMIT 1)|";

    shared::sqlite::SqlStatement stmt(_db, sql.str());
    if (stmt.next()) {
        return getLocalEntryFromSqlStatement(stmt);
    } else {
        return {};
    }
}

std::optional<LocalRecordingIndexEntry> LocalRecordingIndexDb::getEntryForUuid(const std::string& uuid) const {
    std::lock_guard<std::recursive_mutex> guard(_dbMutex);

    std::ostringstream sql;
    sql << R"|(
        SELECT *
        FROM local_vod_entries
        WHERE uuid = ?)|";

    shared::sqlite::SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, uuid);
    if (stmt.next()) {
        return getLocalEntryFromSqlStatement(stmt);
    } else {
        return {};
    }
}

std::vector<LocalRecordingIndexEntry> LocalRecordingIndexDb::getAllLocalEntries() const {
    std::lock_guard<std::recursive_mutex> guard(_dbMutex);
    std::vector<LocalRecordingIndexEntry> entries;

    std::ostringstream sql;
    sql << R"|(
        SELECT *
        FROM local_vod_entries
        ORDER BY cache_time DESC
    )|";

    shared::sqlite::SqlStatement stmt(_db, sql.str());
    while (stmt.next()) {
        entries.push_back(getLocalEntryFromSqlStatement(stmt));
    }
    return entries;
}

void LocalRecordingIndexDb::release() {
    std::lock_guard<std::recursive_mutex> guard(_dbMutex);
    if (_db) {
        sqlite3_close(_db);
    }
    _initFolder.reset();
}

bool changeLocalRecordLocation(const fs::path& from, const fs::path& to) {
    LocalRecordingIndexDb::singleton()->initializeFromFolder(from);
    return LocalRecordingIndexDb::singleton()->moveLocalFolderTo(to);
}

bool cleanupLocalRecordLocation(const fs::path& location, double limit) {
    LocalRecordingIndexDb::singleton()->initializeFromFolder(location);
    return LocalRecordingIndexDb::singleton()->cleanupLocalFolder(limit);
}

}