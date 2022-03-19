#pragma warning(disable: 4996)

#include <boost/program_options.hpp>
#include <iostream>
#include <codecvt>
#include <portaudio.h>
extern "C" {
#include <libavutil/log.h>
}

#include "shared/log/log.h"
#include "vod/vod_clipper.h"

namespace po = boost::program_options;
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
    shared::log::Log::initializeGlobalLogger("manual_clipper.log");
    po::options_description desc("Options");
    desc.add_options()
        ("input", po::value<std::string>()->required(), "Input video to clip")
        ("start", po::value<double>()->required(), "Start timestamp")
        ("end", po::value<double>()->required(), "End timestamp");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    std::cout << "Init FFMpeg Logging" << std::endl;
    av_log_set_level(AV_LOG_VERBOSE);
    av_log_set_callback(ffmpegLogCallback);

    service::vod::VodClipRequest req;
    req.task = "poop";
    req.start = vm["start"].as<double>();
    req.end = vm["end"].as<double>();
    req.source = vm["input"].as<std::string>();
    req.fullCopy = true;
    req.inputFormat = "mp4";
    service::vod::vodClip(req);
    return 0;
}