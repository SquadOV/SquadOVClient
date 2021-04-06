#include "vod/process.h"

#include "shared/filesystem/utility.h"
#include "shared/filesystem/common_paths.h"

#include <cstdlib>
#include <sstream>

namespace fs = std::filesystem;
namespace service::vod {

void processRawLocalVod(const fs::path& from, const fs::path& to) {
    // It's easier to just use the ffmpeg exe directly.
    const fs::path ffmpegPath = shared::filesystem::getCurrentExeFolder() / fs::path("ffmpeg.exe");
    std::ostringstream cmd;
    cmd << 
        shared::filesystem::pathUtf8(ffmpegPath) <<
        " -i " << 
        shared::filesystem::pathUtf8(from) << 
        " -c:v copy -c:a copy -movflags +faststart " << 
        shared::filesystem::pathUtf8(to);
    std::system(cmd.str().c_str());
}

}