#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include "shared/ipc/shared_memory.h"
#include "shared/log/log.h"
#include "shared/process.h"
#include "shared/uuid.h"
#include "shared/filesystem/common_paths.h"
#include "shared/time.h"
#include "shared/base64/decode.h"

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

    std::string rawDemoData(demoUrl->data(), demoUrl->size());
    std::vector<std::string> parts;
    boost::split(parts, rawDemoData, boost::is_any_of("."));

    std::cout << "FOUND DEMO: " << shared::base64::decode(parts[0]) << "@" <<  shared::timeToStr(shared::isoStrToTime(shared::base64::decode(parts[1]))) << std::endl;
    return 0;
}