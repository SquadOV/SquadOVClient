#include <sdkddkver.h>
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include "vod/process.h"

#include "shared/filesystem/utility.h"
#include "shared/filesystem/common_paths.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"

#include <cstdlib>
#include <sstream>

namespace fs = std::filesystem;
namespace bp = boost::process;
namespace service::vod {

void processRawLocalVod(const fs::path& from, const fs::path& to) {
    // It's easier to just use the ffmpeg exe directly.
    const fs::path ffmpegPath = shared::filesystem::getCurrentExeFolder() / fs::path("ffmpeg.exe");
    if (!fs::exists(ffmpegPath)) {
        THROW_ERROR("FFmpeg executable does not exist - reinstallation may be required.");
    }

    boost::asio::io_service ios;
    std::future<std::string> stdoutBuf;
    std::future<std::string> stderrBuf;
    
    bp::child c(
        shared::filesystem::pathUtf8(ffmpegPath),
        "-y",
        "-i", shared::filesystem::pathUtf8(from),
        "-c:v", "copy",
        "-c:a", "copy",
        "-max_muxing_queue_size", "9999",
        "-movflags", "+faststart",
        shared::filesystem::pathUtf8(to),
        bp::std_out > stdoutBuf,
        bp::std_err > stderrBuf,
        ios
    );

    ios.run();
    LOG_INFO("FFMPEG STDOUT: " << stdoutBuf.get() << std::endl);
    LOG_INFO("FFMPEG STDERR: " << stderrBuf.get() << std::endl);
    LOG_INFO("Finished FFmpeg Command: " << c.exit_code() << std::endl);
}

}