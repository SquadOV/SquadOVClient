#include <boost/program_options.hpp>

#include "shared/ipc/shared_memory.h"
#include "shared/log/log.h"
#include "shared/process.h"
#include "shared/uuid.h"
#include "shared/filesystem/common_paths.h"

namespace po = boost::program_options;
namespace fs = std::filesystem;

int main(int argc, char** argv) {
    shared::log::Log::initializeGlobalLogger("shmem_csgo_demo_retriever.log");

    po::options_description desc("Options");
    desc.add_options()
        ("threshold", po::value<std::string>()->required(), "Threshold to pass to demo retriever.");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    const auto segmentId = shared::generateUuidv4();
    const auto strId = shared::generateUuidv4();

    shared::ipc::SharedMemory shmem(segmentId);
    shmem.create(65536);

    auto* demoUrl = shmem.createString(strId);

    const fs::path retriever = shared::filesystem::getCurrentExeFolder() / fs::path("csgo") / fs::path("csgo_demo_retriever.exe");
    std::ostringstream cmd;
    cmd << " --mode steam "
        << " --threshold " << vm["threshold"].as<std::string>()
        << " --shmem \"" << segmentId << "\""
        << " --shvalue \"" << strId << "\"";

    if (shared::process::runProcessWithTimeout(retriever, cmd.str(), std::chrono::seconds(5))) {
        return 1;
    }

    std::string stdDemoUrl(demoUrl->data(), demoUrl->size());
    std::cout << "FOUND DEMO: " << stdDemoUrl << std::endl;
    return 0;
}