#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace shared::wow {

class WtfFile {
public:
    static WtfFile loadFromFile(const std::filesystem::path& path);

    bool hasSetting(const std::string& key) const;
    const std::string& getSetting(const std::string& key) const;
    void WtfFile::setSetting(const std::string& key, const std::string& value);

    void saveToFile(const std::filesystem::path& path) const;
    void addUnparsedLine(const std::string& line);
private:
    std::unordered_map<std::string, std::string> _settings;
    std::vector<std::string> _unparsedLines;
};

}