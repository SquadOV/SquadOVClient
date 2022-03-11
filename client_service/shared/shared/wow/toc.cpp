#include "shared/wow/toc.h"
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <regex>

namespace shared::wow {
const std::regex TAG_REGEX("##\\s*(.+?)\\s*:\\s*(.+)");

TocFile TocFile::loadFromFile(const std::filesystem::path& path) {
    TocFile file;

    std::ifstream f(path);
    std::string line;
    while (std::getline(f, line)) {
        if (boost::algorithm::starts_with(line, "##")) {
            std::smatch matches;
            if (!std::regex_search(line, matches, TAG_REGEX)) {
                continue;
            }

            file.addTag(matches[1].str(), matches[2].str());
        } else if (boost::algorithm::starts_with(line, "#")) {
            // We can ignore comments.
            continue;
        } else {
            boost::algorithm::trim(line);
            file.addFile(line);
        }
    }

    return file;
}

shared::EWowRelease TocFile::wowInterface() const {
    return _interface;
}

std::vector<std::string> TocFile::codeFiles() const {
    return _codeFiles;
}

void TocFile::addTag(const std::string& tag, const std::string& value) {
    if (tag == "Interface") {
        if (boost::algorithm::starts_with(value, "90")) {
            _interface = shared::EWowRelease::Retail;
        } else if (boost::algorithm::starts_with(value, "20")) {
            _interface = shared::EWowRelease::Tbc;
        } else if (boost::algorithm::starts_with(value, "11")) {
            _interface = shared::EWowRelease::Vanilla;
        }
    }
}

void TocFile::addFile(const std::string& file) {
    if (boost::algorithm::ends_with(file, ".lua")) {
        _codeFiles.push_back(file);
    }
}

}