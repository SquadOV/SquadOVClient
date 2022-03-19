#include "vod/process.h"

#include "shared/log/log.h"
#include "shared/errors/error.h"
#include "shared/filesystem/utility.h"
#include "shared/uuid.h"
#include "vod_clipper.h"

namespace fs = std::filesystem;
namespace service::vod {


void processRawLocalVod(const fs::path& from, const fs::path& to) {
    if (!fs::exists(from)) {
        THROW_ERROR("Failed to find input file: " << from);
    }

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