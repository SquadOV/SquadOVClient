#pragma once

#include <filesystem>
#include <unordered_map>

namespace game_event_watcher {

enum class HearthstoneLogSection {
    Decks,
    Zone,
    Power,
    Arena,
    PvpDr,
    Unknown
};

class HearthstoneLogConfig {
public:
    explicit HearthstoneLogConfig(const std::filesystem::path& fname);

    void enableLogSection(HearthstoneLogSection section);
    void load();
    void save();

private:
    struct LogSection {
        int64_t logLevel = 0;
        bool filePrinting = false;
        bool consolePrinting = false;
        bool screenPrinting = false;
        bool verbose = false;

        bool operator==(const LogSection& other) const;
        bool operator!=(const LogSection& other) const;
    };
    std::unordered_map<std::string, LogSection> _sections;
    std::filesystem::path _fname;
    bool _successLoading = false;
    bool _hasChanges = false;
};

}