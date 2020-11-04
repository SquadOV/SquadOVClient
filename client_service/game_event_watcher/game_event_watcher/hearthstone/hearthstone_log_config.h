#pragma once

#include <filesystem>
#include <unordered_map>

namespace game_event_watcher {

enum class HearthstoneLogSection {
    Decks,
    Zone,
    Power
};

class HearthstoneLogConfig {
public:
    explicit HearthstoneLogConfig(const std::filesystem::path& fname);

    void enableLogSection(HearthstoneLogSection section);
    void save();

private:
    struct LogSection {
        int64_t logLevel = 0;
        bool filePrinting = false;
        bool consolePrinting = false;
        bool screenPrinting = false;
    };
    std::unordered_map<HearthstoneLogSection, LogSection> _sections;
    std::filesystem::path _fname;
};

}