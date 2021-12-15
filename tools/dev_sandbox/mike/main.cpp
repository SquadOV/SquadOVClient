#include "shared/log/log.h"
#include "shared/filesystem/common_paths.h"

#include <boost/program_options.hpp>
#include <filesystem>
#include <iostream>
#include <memory>

namespace po = boost::program_options;

int main(int argc, char** argv) {
    shared::log::Log::initializeGlobalLogger("mike_dev_sandbox.log");

    const auto tzDataFolder = shared::filesystem::getSquadOvTzDataFolder();
    date::set_install(tzDataFolder);
    date::get_tzdb_list();

    po::options_description desc("Options");
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    struct Test {
        std::string msg;

        Test():
            msg("Default")
        {

        }

        explicit Test(const std::string& inMsg):
            msg(inMsg)
        {
        }

        ~Test() {
            std::cout << "Destruct: " << msg << std::endl;
        }
    };

    struct Container {
        std::shared_ptr<Test> a;
        std::shared_ptr<Test> b;
        std::shared_ptr<Test> c;
        Test d;

        Container() {
            a = std::make_shared<Test>("A");
            b = std::make_shared<Test>("B");
            c = std::make_shared<Test>("C");
        }
    };

    {
        auto container = std::make_shared<Container>();
        container.reset();
    }

    return 0;
}