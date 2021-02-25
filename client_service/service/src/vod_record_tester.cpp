#pragma warning(disable: 4996)

#include <boost/program_options.hpp>
#include <iostream>
#include <codecvt>
#include <portaudio.h>
extern "C" {
#include <libavutil/log.h>
}

#include "process_watcher/process/process.h"
#include "recorder/game_recorder.h"
#include "shared/log/log.h"

namespace po = boost::program_options;
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
    po::options_description desc("Options");
    desc.add_options()
        ("process", po::value<std::string>()->required(), "Process to record.")
        ("mode", po::value<std::string>()->default_value(""), "VOD mode, DVR or NORMAL")
        ("duration", po::value<int>()->default_value(30), "Length of the final VOD to record")
        ("delay", po::value<int>()->default_value(25), "Delay between when we start DVR and when we want to start recording.")
        ("offset", po::value<int>()->default_value(15), "How many seconds into the DVR we should use in the output VOD")
        ("width", po::value<size_t>()->default_value(1920), "width of output")
        ("height", po::value<size_t>()->default_value(1080), "height of output")
        ("output", po::value<std::string>()->required(), "VOD output file.");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    std::cout << "Init FFMpeg Logging" << std::endl;
    av_log_set_level(AV_LOG_VERBOSE);
    av_log_set_callback(ffmpegLogCallback);
    Pa_Initialize();

    const auto processName = vm["process"].as<std::string>();

    std::vector<process::Process> processes;
    process::listRunningProcesses(processes);

    process::Process finalProcess;
    for (const auto& p : processes) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        const auto pname = conv.to_bytes(p.name());

        if (pname.find(processName) != std::string::npos) {
            finalProcess = p;
            break;
        }
    }

    if (finalProcess.empty()) {
        std::cerr << "Failed to find process." << std::endl;
        return 1;
    }

    const auto outputFname = vm["output"].as<std::string>();
    // Doesn't really matter what game we stick in here yolo.
    service::recorder::GameRecorder recorder(finalProcess, shared::EGame::Aimlab);
    recorder.setFileOutputFromUri("bobbothebuilder", outputFname);
    // Wait for it to grab window info
    std::this_thread::sleep_for(std::chrono::seconds(1));

    const auto mode = vm["mode"].as<std::string>();
    const auto width = vm["width"].as<size_t>();
    const auto height = vm["height"].as<size_t>();
    recorder.overrideResolution(width, height);

    std::thread workerThread;
    if (mode == "NORMAL") {
        const auto duration = vm["duration"].as<int>();
        workerThread = std::thread([&recorder, duration](){
            LOG_INFO("START RECORDING" << std::endl);
            recorder.start(shared::nowUtc(), service::recorder::RecordingMode::Normal, service::recorder::FLAG_WGC_RECORDING);
            std::this_thread::sleep_for(std::chrono::seconds(duration));
            LOG_INFO("STOP RECORDING" << std::endl);
            recorder.stop();
        });
    } else if (mode == "DVR") {
        const auto duration = vm["duration"].as<int>();
        workerThread = std::thread([&recorder, duration](){
            LOG_INFO("START DVR" << std::endl);
            recorder.startDvrSession();
            std::this_thread::sleep_for(std::chrono::seconds(duration));
            LOG_INFO("STOP DVR" << std::endl);
            recorder.stopDvrSession();
            recorder.stopInputs();
        });
    } else if (mode == "DVRJOIN") {
        const auto duration = vm["duration"].as<int>();
        const auto delay = vm["delay"].as<int>();
        const auto offset = vm["offset"].as<int>();
        workerThread = std::thread([&recorder, duration, delay, offset](){
            LOG_INFO("START DVR" << std::endl);
            recorder.startDvrSession();
            std::this_thread::sleep_for(std::chrono::seconds(delay));
            LOG_INFO("DO JOIN" << std::endl);
            recorder.start(shared::nowUtc() - std::chrono::seconds(offset), service::recorder::RecordingMode::DVR);
            std::this_thread::sleep_for(std::chrono::seconds(duration));
            LOG_INFO("STOP RECORDING" << std::endl);
            recorder.stop();
        });
    }

    workerThread.join();
    std::cout << "Output VOD to: " << outputFname << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    Pa_Terminate();
    return 0;
}