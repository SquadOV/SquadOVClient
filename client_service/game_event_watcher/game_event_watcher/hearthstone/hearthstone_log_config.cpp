#include "game_event_watcher/hearthstone/hearthstone_log_config.h"
#include "shared/log/log.h"

#include <algorithm>
#include <cctype>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <fstream>

namespace pt = boost::property_tree;
namespace fs = std::filesystem;
namespace game_event_watcher {
namespace {

const HearthstoneLogSection ALL_LOG_SECTIONS[] = {
    HearthstoneLogSection::Decks,
    HearthstoneLogSection::Power,
    HearthstoneLogSection::Zone,
    HearthstoneLogSection::Arena,
    HearthstoneLogSection::PvpDr
};

std::string logSectionNameToString(HearthstoneLogSection sec) {
    switch (sec) {
        case HearthstoneLogSection::Decks:
            return "Decks";
        case HearthstoneLogSection::Power:
            return "Power";
        case HearthstoneLogSection::Zone:
            return "Zone";
        case HearthstoneLogSection::Arena:
            return "Arena";
        case HearthstoneLogSection::PvpDr:
            return "PVPDR";
    }
    return "";
}

}

HearthstoneLogConfig::HearthstoneLogConfig(const std::filesystem::path& fname):
    _fname(fname) {

    if (fs::exists(_fname)) {
        try {
            LOG_INFO("Load Hearthstone INI: " << _fname.string() << std::endl);
            load();
            _successLoading = true;
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to open Hearthstone INI Config: " << ex.what());
        }
    } else {
        _successLoading = true;
    }
}

bool HearthstoneLogConfig::LogSection::operator==(const LogSection& other) const {
    return logLevel == other.logLevel &&
        filePrinting == other.filePrinting &&
        consolePrinting == other.consolePrinting &&
        screenPrinting == other.screenPrinting &&
        verbose == other.verbose;
}

bool HearthstoneLogConfig::LogSection::operator!=(const LogSection& other) const {
    return !(*this == other);
}

void HearthstoneLogConfig::enableLogSection(HearthstoneLogSection section) {
    const auto key = logSectionNameToString(section);
    const auto oldInfo = _sections[key];
    auto& info = _sections[key];
    info.logLevel = std::max(info.logLevel, int64_t(1));
    info.filePrinting = true;
    // Needs to be here so that we don't make the console printed log too large.
    info.consolePrinting = false;
    _hasChanges |= (oldInfo != info);
}

void HearthstoneLogConfig::load() {
    // Need to write our own parser because HSReplay does some janky stuff and
    // puts in =True/=False into the config file which isn't really handled by
    // most libraries as they expect =true/=false instead. Furthermore, we want
    // to handle potential user errors in the case where multiple log sections exist
    // instead of just erroring out.
    std::ifstream f(_fname.string());
    std::string line;

    std::string currentSection;
    while (std::getline(f, line)) {
        if (line[0] == '[') {
            auto close = line.find(']');
            if (close == std::string::npos) {
                // A bad section?
                throw std::runtime_error("Failed to parse section: " + line);
            }

            currentSection = line.substr(1, close - 1);
            if (currentSection.empty()) {
                throw std::runtime_error("Empty section: " + line);
            }

            // Ignore duplicate sections.
            if (_sections.find(currentSection) != _sections.end()) {
                currentSection = "";
            }
        } else if (!currentSection.empty()) {
            auto it = line.find('=');

            // This will be triggered in the cases of empty lines.
            if (it == std::string::npos) {
                continue;
            }

            std::vector<std::string> tokens;
            boost::split(tokens, line, boost::is_any_of("="));

            if (tokens.size() != 2) {
                throw std::runtime_error("Malformed key value line: " + line);
            }

            std::transform(tokens[1].begin(), tokens[1].end(), tokens[1].begin(), [](unsigned char c) {
                return std::tolower(c);
            });

            auto& data = _sections[currentSection];
            if (tokens[0] == "LogLevel") {
                data.logLevel = std::stoi(tokens[1]);
            } else if (tokens[0] == "FilePrinting") {
                data.filePrinting = (tokens[1] == "true");
            } else if (tokens[0] == "ConsolePrinting") {
                data.consolePrinting = (tokens[1] == "true");
            } else if (tokens[0] == "ScreenPrinting") {
                data.screenPrinting = (tokens[1] == "true");
            } else if (tokens[0] == "Verbose") {
                data.verbose = (tokens[1] == "true");
            } else {
                LOG_WARNING("Unhandled key: " << tokens[0] << std::endl);
            }
        }
    }
}

void HearthstoneLogConfig::save() {
    if (!_successLoading) {
        LOG_WARNING("Failed to load Hearthstone log config so skipping save." << std::endl);
        return;
    }

    if (!_hasChanges) {
        LOG_INFO("No changes necessary for the Hearthstone config - skipping overwrite." << std::endl);
        return;
    }

    // We use Boost's library for writing out a WELL-FORMED INI FILE.
    // Fucking HSReplay :(.
    pt::ptree tree;
    for (const auto& kvp: _sections) {
        tree.put(kvp.first + ".LogLevel", kvp.second.logLevel);
        tree.put(kvp.first + ".FilePrinting", kvp.second.filePrinting);
        tree.put(kvp.first + ".ConsolePrinting", kvp.second.consolePrinting);
        tree.put(kvp.first + ".ScreenPrinting", kvp.second.screenPrinting);
        tree.put(kvp.first + ".Verbose", kvp.second.verbose);
    }

    pt::write_ini(_fname.string(), tree);
}

}