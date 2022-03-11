#include "shared/wow/wtf.h"
#include "shared/log/log.h"
#include <fstream>
#include <regex>

namespace fs = std::filesystem;
namespace shared::wow {
namespace {

const std::regex WTF_SETTING_REGEX("SET\\s+(.+?)\\s+\"(.*)\"");

}

WtfFile WtfFile::loadFromFile(const std::filesystem::path& path) {
    WtfFile output;

    if (fs::exists(path)) {
        std::ifstream f(path);
        std::string line;
        while (std::getline(f, line)) {
            std::smatch matches;
            if (!std::regex_search(line, matches, WTF_SETTING_REGEX)) {
                LOG_WARNING("Can't parse WTF line: " << line << std::endl);
                output.addUnparsedLine(line);
                continue;
            }

            output.setSetting(matches[1].str(), matches[2].str());
        }
    }

    return output;
}

bool WtfFile::hasSetting(const std::string& key) const {
    return _settings.find(key) != _settings.end();
}

const std::string& WtfFile::getSetting(const std::string& key) const {
    return _settings.at(key);
}

void WtfFile::setSetting(const std::string& key, const std::string& value) {
    _settings[key] = value;
}

void WtfFile::addUnparsedLine(const std::string& line) {
    _unparsedLines.push_back(line);
}

void WtfFile::saveToFile(const std::filesystem::path& path) const {
    fs::create_directories(path.parent_path());

    std::ofstream of(path, std::ios_base::out | std::ios_base::trunc);
    for (const auto& kvp: _settings) {
        of << "SET " << kvp.first << " \"" << kvp.second << "\"" << std::endl;
    }
    of.close();
}

}