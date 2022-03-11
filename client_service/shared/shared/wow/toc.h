#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include "shared/games.h"

namespace shared::wow {

class TocFile {
public:
    static TocFile loadFromFile(const std::filesystem::path& path);

    shared::EWowRelease wowInterface() const;
    std::vector<std::string> codeFiles() const;

    void addTag(const std::string& tag, const std::string& value);
    void addFile(const std::string& file);

private:
    shared::EWowRelease _interface;
    std::vector<std::string> _codeFiles;
};

}