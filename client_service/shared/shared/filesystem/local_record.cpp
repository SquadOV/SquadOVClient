#include "shared/filesystem/local_record.h"
#include "shared/filesystem/utility.h"
#include "shared/filesystem/common_paths.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"
#include "shared/sqlite/sqlite.h"
#include "shared/time.h"
#include "shared/http/http_client.h"
#include "shared/uuid.h"
#include "shared/strings/strings.h"

#include <boost/algorithm/string.hpp>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/files.h>
#include <cryptopp/md5.h>
#include <cryptopp/base64.h>
#include <ShObjIdl.h>
#include <objbase.h>
#include <propvarutil.h>
#include <functional>
#include <wil/com.h>

using namespace std::chrono_literals;
namespace fs = std::filesystem;
namespace shared::filesystem {
namespace {

PROPERTYKEY getSquadOvVideoPropertyKey() {
    PROPERTYKEY key = { 0 };
    IIDFromString(L"{2adb913e-d026-4198-9931-a737d1e78fc3}", &key.fmtid);
    key.pid = PID_FIRST_USABLE;
    return key;
}

constexpr auto BUFFER_NUM_BYTES = 16384;

}

std::filesystem::path LocalRecordingIndexEntry::fullPath() const {
    return root / relative;
}

LocalRecordingIndexDb* LocalRecordingIndexDb::singleton() {
    static LocalRecordingIndexDbPtr s = std::make_unique<LocalRecordingIndexDb>();
    return s.get();
}

LocalRecordingIndexDb::LocalRecordingIndexDb() {
}

LocalRecordingIndexDb::~LocalRecordingIndexDb() {
}

void LocalRecordingIndexDb::initializeFromFolder(const fs::path& parentFolder) {
    const auto absParentFolder = fs::absolute(parentFolder);
    if (_initFolder.has_value() && _initFolder.value() == absParentFolder) {
        return;
    }

    if (!fs::exists(absParentFolder)) {
        fs::create_directories(absParentFolder);   
    }

    _initFolder = absParentFolder;
    migrateToV2();
    rebuildDatabase();
}

void LocalRecordingIndexDb::migrateToV2() const {
    if (!_initFolder) {
        return;
    }

    LOG_INFO("Executing migration of local storage to V2..." << std::endl);
    // We previously used an SQLite database to index what videos a user stored on their machine. This system sucked so much.
    // Migrate to our new system where we build up the database ourself based on videos we find in the folder.
    const auto indexFile = _initFolder.value() / fs::path("index.db");
    if (!fs::exists(indexFile)) {
        LOG_INFO("...Already migrated." << std::endl);
        return;
    }

    // Absolutely ditch the Sqlite database and ignore what's in it since we can rebuild everything we need from what's on the
    // user's filesystem. Quite simply, within _initFolder, there will be a bunch of folders with UUID names and within them, a file called
    // video.mp4. If such a format exists, then we can assume that the video.mp4 is some video stored in our server's database as well.
    // Thus, we're able to find all videos that need to be migrated.
    try {
        for (const auto& entry : fs::directory_iterator(_initFolder.value())) {
            if (!entry.is_directory()) {
                continue;
            }

            LOG_INFO("Checking Migration Folder: " << entry << std::endl);
            const std::string videoUuid = shared::filesystem::pathUtf8(entry.path().filename());
            if (!shared::isValidUuid(videoUuid)) {
                LOG_INFO("...Invalid UUID: " << videoUuid << std::endl);
                continue;
            }

            const fs::path videoFile = entry / fs::path("video.mp4");
            if (!fs::exists(videoFile)) {
                LOG_INFO("...Video File Doesn't Exist." << std::endl);
                continue;
            }

            // The migration is simple - simply add the metadata to the video file in the way we'd expect.
            if (!addMetadataToFile(videoFile, videoUuid)) {
                LOG_WARNING("...Failed to add metadata to file: " << videoFile << std::endl);
            }
        }
    } catch (std::exception& ex) {
        // I mean...if there's an error here then chances are we'll never be able to migrate them properly.
        LOG_WARNING("...Something went wrong trying to migrate videos: " << ex.what() << std::endl);
    }

    fs::remove(indexFile);
}

bool LocalRecordingIndexDb::addMetadataToFile(const std::filesystem::path& path, const std::string& videoUuid) const {
    const auto absPath = fs::absolute(path);
    wil::com_ptr<IPropertyStore> props = nullptr;
    HRESULT hr = SHGetPropertyStoreFromParsingName(
        absPath.native().c_str(),
        nullptr,
        GPS_READWRITE,
        IID_PPV_ARGS(&props)
    );

    if (hr != S_OK) {
        LOG_WARNING("Failed to get prop store [rw] for: " << absPath << std::endl);
        return false;
    }

    std::wstring wideVideoUuid = shared::strings::utf8ToWcs(videoUuid);

    PROPVARIANT value = {0};
    hr = InitPropVariantFromString(wideVideoUuid.c_str(), &value);
    if (hr != S_OK) {
        LOG_WARNING("Failed to init prop value: " << absPath << std::endl);
        return false;
    }

    hr = props->SetValue(getSquadOvVideoPropertyKey(), value);
    if (hr != S_OK) {
        LOG_WARNING("Failed to set prop value: " << absPath << std::endl);
        return false;
    }

    hr = props->Commit();
    if (hr != S_OK) {
        LOG_WARNING("Failed to commit prop value to filesystem: " << absPath << std::endl);
        return false;
    }
    return true;
}

void LocalRecordingIndexDb::rebuildDatabase() {
    if (!_initFolder) {
        LOG_ERROR("Rebuilding database without a folder specified." << std::endl);
        return;
    }

    LOG_INFO("Rebuilding Local Database...:" << _initFolder.value() << std::endl);
    {
        std::lock_guard guard(_dbMutex);
        _uuidDatabase.clear();
        _pathDatabase.clear();
    }

    for (const auto& entry : fs::recursive_directory_iterator(_initFolder.value())) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const auto dbEntry = buildEntryFromFile(entry);
        if (!dbEntry) {
            continue;
        }

        addEntryToDatabase(dbEntry.value());
    }

    LOG_INFO("Restarting watch thread..." << std::endl);
    _watchRunning = false;
    if (_watchThread.joinable()) {
        _watchThread.join();
    }
    _watchRunning = true;
    _watchThread = std::thread([this](){
        HANDLE hDir = CreateFileW(     
            _initFolder.value().native().c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL
        );

        std::unique_ptr<BYTE, decltype(&_aligned_free)> buffer((BYTE*)_aligned_malloc(BUFFER_NUM_BYTES, sizeof(BYTE)), _aligned_free);
        while (_watchRunning) {
            OVERLAPPED overlap = { 0 };
            BOOL ok = ReadDirectoryChangesW(
                hDir,
                buffer.get(),
                BUFFER_NUM_BYTES,
                TRUE,
                FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES,
                nullptr,
                &overlap,
                nullptr
            );

            if (!ok) {
                LOG_WARNING("Failed to setup incremental update watch: " << _initFolder.value() << std::endl);
                std::this_thread::sleep_for(128ms);
                continue;
            }

            DWORD bytesTransferred = 0;

            while (_watchRunning) {
                if (GetOverlappedResultEx(hDir, &overlap, &bytesTransferred, 200, TRUE)) {
                    BYTE* rawNotif = buffer.get();
                    while (!!rawNotif && _watchRunning) {
                        FILE_NOTIFY_INFORMATION* notif = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(rawNotif);

                        const std::wstring wNotifFname(notif->FileName, notif->FileNameLength / sizeof(wchar_t));
                        const fs::path wNotifPath = _initFolder.value() / fs::path(wNotifFname);
                        if (notif->Action == FILE_ACTION_REMOVED || notif->Action == FILE_ACTION_RENAMED_OLD_NAME) {
                            LOG_INFO("[DB UPDATE] Removed/Renamed File: " << wNotifPath << std::endl);
                            removeFileFromDatabase(wNotifPath);
                        } else {
                            LOG_INFO("[DB UPDATE] New/Updated File: " << wNotifPath << std::endl);
                            const auto newEntry = buildEntryFromFile(wNotifPath);
                            if (newEntry) {
                                addEntryToDatabase(newEntry.value());
                            }
                        }

                        if (notif->NextEntryOffset == 0) {
                            break;
                        }
                        rawNotif += notif->NextEntryOffset;
                    }
                    break;
                }
            }
            
            std::this_thread::sleep_for(16ms);
        }

        CloseHandle(hDir);
    });
}

void LocalRecordingIndexDb::addEntryToDatabase(const LocalRecordingIndexEntry& entry) {
    std::lock_guard guard(_dbMutex);
    _uuidDatabase[entry.uuid] = entry;
    _pathDatabase[shared::filesystem::pathUtf8(entry.relative)] = entry;
}

void LocalRecordingIndexDb::removeVideoFromDatabase(const std::string& uuid) {
    const auto toRemove = getEntryForUuid(uuid);
    if (toRemove) {
        removeEntryFromDatabase(toRemove.value());
    }
}

void LocalRecordingIndexDb::removeFileFromDatabase(const std::filesystem::path& path) {
    LocalRecordingIndexEntry toRemove;

    {
        std::shared_lock guard(_dbMutex);
        const auto absPath = fs::absolute(path);
        const auto relPath = fs::relative(absPath, _initFolder.value());

        const auto it = _pathDatabase.find(shared::filesystem::pathUtf8(relPath));
        if (it == _pathDatabase.end()) {
            return;
        }

        toRemove = it->second;
    }

    removeEntryFromDatabase(toRemove);
}

void LocalRecordingIndexDb::removeEntryFromDatabase(const LocalRecordingIndexEntry& entry) {
    std::lock_guard guard(_dbMutex);

    {
        const auto it = _pathDatabase.find(shared::filesystem::pathUtf8(entry.relative));
        if (it != _pathDatabase.end()) {
            _pathDatabase.erase(it);
        }
    }

    {
        const auto it = _uuidDatabase.find(entry.uuid);
        if (it != _uuidDatabase.end()) {
            _uuidDatabase.erase(it);
        }
    }

    if (fs::exists(entry.fullPath())) {
        fs::remove_all(entry.fullPath());
    }
}

std::optional<LocalRecordingIndexEntry> LocalRecordingIndexDb::buildEntryFromFile(const std::filesystem::path& path) const {
    const auto absPath = fs::absolute(path);
    wil::com_ptr<IPropertyStore> props = nullptr;
    HRESULT hr = SHGetPropertyStoreFromParsingName(
        absPath.native().c_str(),
        nullptr,
        GPS_DEFAULT,
        IID_PPV_ARGS(&props)
    );

    if (hr != S_OK) {
        LOG_WARNING("Failed to get prop store [build] for: " << absPath << std::endl);
        return std::nullopt;
    }

    PROPERTYKEY key = getSquadOvVideoPropertyKey();
    PROPVARIANT value = { 0 };
    hr = props->GetValue(key, &value);
    if (hr != S_OK) {
        LOG_WARNING("Failed to get prop value for: " << absPath << std::endl);
        return std::nullopt;
    }

    if (!IsPropVariantString(value)) {
        LOG_WARNING("Found file with invalid prop variant:" << absPath << std::endl);
        return std::nullopt;
    }

    std::wstring videoUuid(value.pwszVal);
    std::string utf8VideoUuid = shared::strings::wcsToUtf8(videoUuid);
    if (!shared::isValidUuid(utf8VideoUuid)) {
        LOG_WARNING("Found Invalid UUID: " << absPath << "\t" << utf8VideoUuid << std::endl);
        return std::nullopt;
    }

    LocalRecordingIndexEntry ret;
    ret.uuid = utf8VideoUuid;
    ret.root = _initFolder.value();
    ret.relative = fs::relative(absPath, _initFolder.value());
    ret.diskBytes = fs::file_size(absPath);
    ret.lastWriteTime = shared::filesystem::timeOfLastFileWrite(absPath);
    return ret;
}

bool LocalRecordingIndexDb::moveLocalFolderTo(const fs::path& to) {
    if (!_initFolder.has_value()) {
        LOG_WARNING("Attempting to migrate local recording folder when index has not been initialized." << std::endl);
        return true;
    }

    const auto absTo = fs::absolute(to);
    if (fs::canonical(absTo.root_path()) == fs::canonical(absTo)) {
        LOG_WARNING("Can not switch local recording location to a root path: " << absTo << std::endl);
        return false;
    }

    // (source , destination)
    std::vector<std::pair<fs::path, fs::path>> migrationTasks;
    std::vector<std::pair<fs::path, fs::path>> completedTasks;

    // Stop the watch first or else the delete at the end gets picked up.
    _watchRunning = false;
    if (_watchThread.joinable()) {
        _watchThread.join();
    }

    {
        std::shared_lock guard(_dbMutex);
        for (const auto& kvp: _uuidDatabase) {
            const auto from = kvp.second.root / kvp.second.relative;
            const auto to = absTo / kvp.second.relative;
            migrationTasks.push_back(std::make_pair(from, to));
        }
    }

    // Now that we've tracked all the source/destination files, we want to go through and start copying
    // the files to their new destination. If this all succeeds, then great, remove the old files. If it fails,
    // we want to remove the files we copied over already.
    try {
        for (const auto& task: migrationTasks) {
            if (!fs::exists(task.first)) {
                continue;
            }
            fs::create_directories(task.second.parent_path());
            fs::copy_file(task.first, task.second, fs::copy_options::overwrite_existing);
            completedTasks.push_back(task);
        }
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to move local folder: " << ex.what() << std::endl);
        try {
            for (const auto& task: completedTasks) {
                fs::remove_all(task.second);
            }
        } catch (std::exception& ex2) {
            LOG_ERROR("MIGRATION FAILURE RECOVERY FAILED [INITIAL]: " << ex2.what() << std::endl);
        }
        return false;
    }

    completedTasks.clear();

    // Now do another run where we delete the original videos. If this fails then this run fails.
    // We *MUST* cleanup properly as well.
    const auto oldFolder = _initFolder.value();
    try {
        for (const auto& task: migrationTasks) {
            if (!fs::exists(task.first)) {
                continue;
            }
            fs::remove_all(task.first);
            completedTasks.push_back(task);
        }

        // If the initialization fails then we still want to clean up as well.
        initializeFromFolder(absTo);
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to delete original video(s): " << ex.what() << std::endl);
        try {
            for (const auto& task: completedTasks) {
                fs::copy_file(task.second, task.first, fs::copy_options::overwrite_existing);
            }

            for (const auto& task: migrationTasks) {
                fs::remove_all(task.second);
            }
        } catch (std::exception& ex2) {
            LOG_ERROR("MIGRATION FAILURE RECOVERY FAILED [CLEANUP]: " << ex2.what() << std::endl);
        }

        initializeFromFolder(oldFolder);
        return false;
    }
     
    return true;
}

size_t LocalRecordingIndexDb::currentSizeBytes() const {
    std::shared_lock guard(_dbMutex);
    size_t dbSize = 0;
    for (const auto& kvp: _uuidDatabase) {
        dbSize += kvp.second.diskBytes;
    }
    return dbSize;
}

bool LocalRecordingIndexDb::cleanupLocalFolder(double limit) {
    if (!_initFolder.has_value()) {
        LOG_WARNING("Attempting to cleanup local recording folder when index has not been initialized." << std::endl);
        return true;
    }

    try {
        double currentSize = currentSizeBytes() / 1024.0 / 1024.0 / 1024.0;
        while (currentSize > limit) {
            const auto entry = getOldestLocalEntry();
            if (!entry) {
                break;
            }

            const auto val = entry.value();
            removeVideoFromDatabase(val.uuid);
            currentSize -= val.diskBytes / 1024.0 / 1024.0 / 1024.0;
        }

        return true;
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to cleanup local entries: " << ex.what() << std::endl);
        return false;
    }
    return true;
}

void LocalRecordingIndexDb::addLocalEntryFromUri(const std::string& uri, const std::string& md5Checksum, const LocalRecordingIndexEntry& entry, const shared::http::DownloadUploadProgressFn& progressFn) {
    const auto dlPath = shared::filesystem::getSquadOvTempFolder()  / fs::path(entry.relative);
    fs::create_directories(dlPath.parent_path());

    bool success = false;
    for (int tries = 0; tries < 3 && !success; ++tries) {
        LOG_INFO("Try " << tries << " to download: " << uri << std::endl);
        fs::remove(dlPath);

        shared::http::HttpClient client(uri);
        client.addDownloadProgressFn(progressFn);
        const auto resp = client.download("", dlPath);
        if (resp->status != 200) {
            THROW_ERROR("Failed to download URI: " << resp->body << std::endl);
        }

        // If we are given a valid checksum then we need to check to make sure that the file we downloaded
        // has the correct md5 checksum to ensure file integrity.
        if (!md5Checksum.empty()) {
            CryptoPP::Weak::MD5 hash;
            std::string digest;
            
            CryptoPP::FileSource file(
                dlPath.c_str(),
                true,
                new CryptoPP::HashFilter(hash, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(digest)))
            );

            boost::algorithm::trim(digest);
            success = boost::iequals(digest, md5Checksum);
            LOG_INFO("...Comparing MD5 :: " << digest << " [Computed] vs " << md5Checksum << " [Reference]...Success - " << success << std::endl);
        } else {
            success = true;
            break;
        }
    }

    if (!success) {
        THROW_ERROR("Failed to download VOD (md5?)");
        return;
    }

    addLocalEntryFromFilesystem(dlPath, entry);
    fs::remove_all(dlPath);
}

void LocalRecordingIndexDb::addLocalEntryFromFilesystem(const std::filesystem::path& file, const LocalRecordingIndexEntry& entry) {
    addMetadataToFile(file, entry.uuid);

    LocalRecordingIndexEntry canonicalEntry = entry;
    canonicalEntry.root = _initFolder.value();
    canonicalEntry.lastWriteTime = shared::filesystem::timeOfLastFileWrite(file);
    canonicalEntry.diskBytes = fs::file_size(file);

    const auto outputPath = canonicalEntry.fullPath();
    const auto parentDir = outputPath.parent_path();
    fs::create_directories(parentDir);

    bool failure = false;
    try {
        fs::copy(file, outputPath);
        LOG_INFO("Added local entry to DB: " << outputPath << std::endl);
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to create local entry: " << ex.what() << std::endl);
        failure = true;
        return;
    }

    if (!failure) {
        addEntryToDatabase(canonicalEntry);
    } else {
        fs::remove_all(parentDir);
    }

    fs::remove(file);
}

std::optional<LocalRecordingIndexEntry> LocalRecordingIndexDb::getOldestLocalEntry() const {
    std::shared_lock guard(_dbMutex);

    shared::TimePoint oldestTime = shared::nowUtc();
    std::optional<LocalRecordingIndexEntry> oldestEntry;

    for (const auto& kvp: _uuidDatabase) {
        if (kvp.second.lastWriteTime < oldestTime) {
            oldestTime = kvp.second.lastWriteTime;
            oldestEntry = kvp.second;
        }
    }

    return oldestEntry;
}

std::optional<LocalRecordingIndexEntry> LocalRecordingIndexDb::getEntryForUuid(const std::string& uuid) const {
    std::shared_lock guard(_dbMutex);

    const auto it = _uuidDatabase.find(uuid);
    if (it == _uuidDatabase.end()) {
        return std::nullopt;
    }

    return it->second;
}

std::vector<LocalRecordingIndexEntry> LocalRecordingIndexDb::getAllLocalEntries() const {
    std::shared_lock guard(_dbMutex);

    std::vector<LocalRecordingIndexEntry> ret;
    ret.reserve(_uuidDatabase.size());

    for (const auto& kvp: _uuidDatabase) {
        ret.push_back(kvp.second);
    }

    std::sort(ret.begin(), ret.end(), [](const LocalRecordingIndexEntry& a, const LocalRecordingIndexEntry& b){
        return a.lastWriteTime > b.lastWriteTime;
    });

    return ret;
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