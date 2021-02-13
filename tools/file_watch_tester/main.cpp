#include "shared/log/log.h"
#include "game_event_watcher/logs/log_watcher.h"

#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace po = boost::program_options;
namespace fs = std::filesystem;
using namespace game_event_watcher;

int main(int argc, char** argv) {
    po::options_description desc("Options");
    desc.add_options()
        ("file", po::value<std::string>()->required(), "File to watch for changes.");
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    const fs::path filePath(vm["file"].as<std::string>());    
    LogWatcher watcher(
        filePath,
        [](const std::vector<std::string>& lines){
            for (const auto& l : lines) {
                std::cout << l << std::endl;
            }
        },
        false,
        true
    );
    watcher.disableBatching();
    watcher.wait();
    return 0;
}