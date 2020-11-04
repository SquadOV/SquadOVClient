#include "game_event_watcher/hearthstone/hearthstone_log_config.h"

#include <algorithm>
#include <fstream>
#include <toml++/toml.h>

namespace fs = std::filesystem;
namespace game_event_watcher {
namespace {

const HearthstoneLogSection ALL_LOG_SECTIONS[] = {
    HearthstoneLogSection::Decks,
    HearthstoneLogSection::Power,
    HearthstoneLogSection::Zone
};

std::string logSectionNameToString(HearthstoneLogSection sec) {
    switch (sec) {
        case HearthstoneLogSection::Decks:
            return "Decks";
        case HearthstoneLogSection::Power:
            return "Power";
        case HearthstoneLogSection::Zone:
            return "Zone";
    }
    return "";
}

}

HearthstoneLogConfig::HearthstoneLogConfig(const std::filesystem::path& fname):
    _fname(fname) {

    if (fs::exists(_fname)) {
        const toml::table tbl = toml::parse_file(_fname.string());
        for (const auto& secKey : ALL_LOG_SECTIONS) {
            const auto sec = tbl[logSectionNameToString(secKey)];
            if (!sec) {
                continue;
            }

            LogSection newSec;
            newSec.logLevel = sec["LogLevel"].value<int64_t>().value_or(0);
            newSec.filePrinting = sec["FilePrinting"].value<bool>().value_or(false);
            newSec.consolePrinting = sec["ConsolePrinting"].value<bool>().value_or(false);
            newSec.screenPrinting = sec["ScreenPrinting"].value<bool>().value_or(false);
            _sections[secKey] = newSec;
        }
    }
}

void HearthstoneLogConfig::enableLogSection(HearthstoneLogSection section) {
    auto info = _sections[section];
    info.logLevel = std::max(info.logLevel, int64_t(1));
    info.consolePrinting = true;
    _sections[section] = info;
}

void HearthstoneLogConfig::save() {
    // Re-read the file because we might not have parsed *everything* as we
    // want to maintain the user's previous configs as best as we can.
    toml::table tbl = fs::exists(_fname) ? toml::parse_file(_fname.string()) : toml::table();
    for (const auto& secKey : ALL_LOG_SECTIONS) {
        const auto secName = logSectionNameToString(secKey);
        const auto& info = _sections[secKey];

        const auto sec = toml::table{{
            { "LogLevel", info.logLevel },
            { "FilePrinting", info.filePrinting },
            { "ConsolePrinting", info.consolePrinting },
            { "ScreenPrinting", info.screenPrinting }
        }};
        tbl.insert_or_assign(secName, sec);
    }

    std::ofstream output(_fname.string());
    output << tbl;
}

}