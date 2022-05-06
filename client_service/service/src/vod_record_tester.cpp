#pragma warning(disable: 4996)

#include <boost/program_options.hpp>
#include <filesystem>
#include <iostream>
#include <codecvt>
#include <condition_variable>
#include <portaudio.h>
extern "C" {
#include <libavutil/log.h>
}

#include "process_watcher/process/process.h"
#include "recorder/game_recorder.h"
#include "shared/log/log.h"
#include "api/squadov_api.h"
#include "shared/env.h"
#include "system/win32/message_loop.h"
#include "shared/system/win32/interfaces/win32_system_process_interface.h"
#include "shared/system/win32/process.h"
#include "vod/process.h"

namespace po = boost::program_options;
namespace fs = std::filesystem;
using namespace process_watcher;
void ffmpegLogCallback(void* ptr, int level, const char* fmt, va_list v1) {
    if (level > av_log_get_level()) {
        return;
    }

    char buffer[2048];
    vsprintf(buffer, fmt, v1);

    std::string sBuffer(buffer);
    std::cout << sBuffer << std::endl;
}

int main(int argc, char** argv) {
    shared::log::Log::initializeGlobalLogger("vod_record_tester.log");
#ifdef _WIN32
    // I think this is needed because we aren't generally calling startRecording on the same thread as Pa_Initialize?
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif

    shared::system::win32::elevateProcessPriority(ABOVE_NORMAL_PRIORITY_CLASS);

    bool fastify = false;
    po::options_description desc("Options");
    desc.add_options()
        ("process", po::value<std::string>()->required(), "Process to record.")
        ("mode", po::value<std::string>()->default_value(""), "VOD mode, DVR or NORMAL")
        ("duration", po::value<int>()->default_value(30), "Length of the final VOD to record")
        ("delay", po::value<int>()->default_value(25), "Delay between when we start DVR and when we want to start recording.")
        ("offset", po::value<int>()->default_value(15), "How many seconds into the DVR we should use in the output VOD")
        ("loop", po::value<int>()->default_value(1), "Number of times to loop DVR recording")
        ("width", po::value<size_t>()->default_value(1920), "width of output")
        ("height", po::value<size_t>()->default_value(1080), "height of output")
        ("output", po::value<std::string>()->required(), "VOD output file.")
        ("fastify", po::bool_switch(&fastify), "Do fastify");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    std::cout << "Init FFMpeg Logging" << std::endl;
    av_log_set_level(AV_LOG_VERBOSE);
    av_log_set_callback(ffmpegLogCallback);

    service::api::getGlobalApi()->setSessionId(shared::getEnv("SQUADOV_SESSION_ID"));

    const auto processName = vm["process"].as<std::string>();

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wProcessName = converter.from_bytes(processName);

    process::ProcessRunningState processes(std::make_shared<shared::system::win32::interfaces::Win32SystemProcessInterface>());
    processes.update();
    const auto allProcesses = processes.getProcesssRunningByName(wProcessName, false);

    if (allProcesses.empty()) {
        std::cerr << "Failed to find process." << std::endl;
        return 1;
    }

    const auto outputFname = vm["output"].as<std::string>();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    // Doesn't really matter what game we stick in here yolo.
    service::recorder::GameRecorder recorder(allProcesses[0], shared::EGame::Hearthstone);
    recorder.loadCachedInfo();
    recorder.addManualGameAudioProcess("RiotClientServices.exe");

    service::uploader::UploadDestination destination;
    destination.loc = service::uploader::UploadManagerType::FileSystem;
    destination.purpose = service::uploader::UploadPurpose::VOD;
    destination.url = outputFname;
    recorder.setFileOutputFromDestination("bobbothebuilder", destination);
    // Wait for it to grab window info
    std::this_thread::sleep_for(std::chrono::seconds(1));

    const auto mode = vm["mode"].as<std::string>();
    const auto width = vm["width"].as<size_t>();
    const auto height = vm["height"].as<size_t>();
    const auto loop = vm["loop"].as<int>();
    //recorder.overrideResolution(width, height);

    std::thread workerThread;
    if (mode == "NORMAL") {
        const auto duration = vm["duration"].as<int>();
        workerThread = std::thread([&recorder, duration](){
            LOG_INFO("START RECORDING" << std::endl);
            recorder.start(service::recorder::FLAG_WGC_RECORDING);
            recorder.initializeVideoOutput(shared::nowUtc(), service::recorder::RecordingMode::Normal);
            std::this_thread::sleep_for(std::chrono::seconds(duration));
            LOG_INFO("STOP RECORDING" << std::endl);
            recorder.stop({}, true);
        });
    } else if (mode == "DVRJOIN") {
        const auto duration = vm["duration"].as<int>();
        const auto delay = vm["delay"].as<int>();
        const auto offset = vm["offset"].as<int>();
        workerThread = std::thread([&recorder, duration, delay, offset, loop, destination](){
            for (auto i = 0; i < loop; ++i) {
                std::stringstream str;
                str << "bobbothebuilder" << i;
                recorder.setFileOutputFromDestination(str.str(), destination);
                
                LOG_INFO("START DVR" << std::endl);
                recorder.start(service::recorder::FLAG_WGC_RECORDING);
                recorder.initializeDvrOutput(120.0);
                std::this_thread::sleep_for(std::chrono::seconds(delay));
                LOG_INFO("DO JOIN" << std::endl);
                recorder.initializeVideoOutput(shared::nowUtc() - std::chrono::seconds(offset), service::recorder::RecordingMode::DVR);
                std::this_thread::sleep_for(std::chrono::seconds(duration));
                LOG_INFO("STOP RECORDING" << std::endl);
                recorder.stop({}, true);
            }
        });
    }

    std::cout << "Output VOD to: " << outputFname << std::endl;
    //shared::system::win32::Win32MessageLoop::singleton()->start();
    if (workerThread.joinable()) {
        workerThread.join();
    }

    if (fastify) {
        const auto fromPath = fs::path(outputFname);
        auto toPath = fromPath;
        toPath.replace_extension(fs::path("mp4"));
        service::vod::processRawLocalVod(fromPath, toPath);
    }

    return 0;
}