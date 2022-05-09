#include <sdkddkver.h>
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <boost/algorithm/string.hpp>
#include "vod/process.h"

#include "shared/log/log.h"
#include "shared/errors/error.h"
#include "shared/filesystem/utility.h"
#include "shared/filesystem/common_paths.h"
#include "shared/env.h"
#include "shared/uuid.h"
#include "vod_clipper.h"

namespace fs = std::filesystem;
namespace bp = boost::process;
namespace service::vod {


void processRawLocalVod(const fs::path& from, const fs::path& to) {
    if (!fs::exists(from)) {
        THROW_ERROR("Failed to find input file: " << from);
    }

    // It's easier to just use the ffmpeg exe directly.
    fs::path ffmpegPath = std::filesystem::path(shared::getEnv("FFMPEG_BINARY_PATH", ""));
    if (!fs::exists(ffmpegPath)) {
        ffmpegPath = shared::filesystem::getCurrentExeFolder() / fs::path("ffmpeg.exe");
    }
    
    if (!fs::exists(ffmpegPath)) {
        THROW_ERROR("FFmpeg executable does not exist - reinstallation may be required.");
    }

    LOG_INFO("Launching FFmpeg: " << ffmpegPath << std::endl);
    boost::asio::io_service ios;
    std::future<std::string> stdoutBuf;
    std::future<std::string> stderrBuf;
    
    bp::child c(
        ffmpegPath.native(),
        "-y",
        "-analyzeduration", "100M",
        "-probesize", "100M",
        "-i", from.native(),
        "-c:v", "copy",
        "-c:a", "copy",
        "-max_muxing_queue_size", "9999",
        "-movflags", "+faststart",
        to.native(),
        bp::std_out > stdoutBuf,
        bp::std_err > stderrBuf,
        ios
    );

    ios.run();
    c.wait();

    const auto stdoutStr = boost::algorithm::trim_copy(stdoutBuf.get());
    const auto stderrStr = boost::algorithm::trim_copy(stderrBuf.get());
    LOG_INFO("FFMPEG STDOUT: " << stdoutStr << std::endl);
    LOG_INFO("FFMPEG STDERR: " << stderrStr << std::endl);
    LOG_INFO("Finished FFmpeg Command: " << c.exit_code() << " " << c.native_exit_code() << std::endl);

    if (!fs::exists(to)) {
        LOG_INFO("...Falling back to manual processing via FFMPEG C API." << std::endl);
        try {
            VodClipRequest req;
            req.source = shared::filesystem::pathUtf8(from);
            req.task = shared::generateUuidv4();
            req.fullCopy = true;
            req.inputFormat = "mpegts";

            const auto resp = vodClip(req);
            if (!fs::exists(fs::path(resp.path))) {
                THROW_ERROR("Failed to generate processed VOD: " << resp.path);
                return;
            }

            fs::copy(fs::path(resp.path), to);
            fs::remove(fs::path(resp.path));
        } catch (std::exception& ex) {
            LOG_ERROR("Failed to process local VOD: " << ex.what() << std::endl);
        }
    }
}

}