#include "shared/squadov/vod.h"
#include "shared/time.h"
#include "shared/filesystem/utility.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}


namespace shared::squadov {
namespace {

struct InputVideoFile {
    AVFormatContext* formatContext = nullptr;

    explicit InputVideoFile(const std::filesystem::path& fname);
    ~InputVideoFile();

    VodMetadata metadata() const;
};

}

InputVideoFile::InputVideoFile(const std::filesystem::path& fname) {
    const auto fnameUtf8 = shared::filesystem::pathUtf8(fname);

    // Do a quick open using FFmpeg to get all this data we need.
    AVDictionary* options = nullptr;

    // To parallel what we have setup for video processing already.
    av_dict_set(&options, "analyzeduration", "100000000", 0);
    av_dict_set(&options, "probesize", "100000000", 0);
    
    if (avformat_open_input(&formatContext, fnameUtf8.c_str(), nullptr, nullptr) < 0) {
        av_dict_free(&options);
        THROW_ERROR("Failed to open input video file.");
        return;
    }

    av_dict_free(&options);
}

VodMetadata InputVideoFile::metadata() const {
    VodMetadata data;

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        THROW_ERROR("Failed to find stream info.");
        return data;
    }

    if (formatContext->nb_streams == 0) {
        THROW_ERROR("No input streams.");
        return data;
    }

    const auto* vstream = formatContext->streams[0];
    if (vstream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) {
        THROW_ERROR("0-index stream is not a video. Did we make this?");
        return data;
    }

    const auto* codecContext = vstream->codec;

    data.resX = codecContext->width;
    data.resY = codecContext->height;
    data.fps = static_cast<double>(vstream->avg_frame_rate.num) / vstream->avg_frame_rate.den;
    data.avgBitrate = codecContext->bit_rate;
    data.minBitrate = data.avgBitrate;
    data.maxBitrate = data.avgBitrate;

    return data;
}

InputVideoFile::~InputVideoFile() {
    if (formatContext) {
        avformat_close_input(&formatContext);
    }
}


nlohmann::json VodAssociation::toJson() const {
    return {
        { "matchUuid", matchUuid },
        { "userUuid", userUuid },
        { "videoUuid", videoUuid },
        { "startTime", shared::timeToIso(startTime) },
        { "endTime", shared::timeToIso(endTime  ) },
        { "rawContainerFormat", rawContainerFormat },
        { "isClip" , isClip },
        { "isLocal", isLocal }
    };
}

VodAssociation VodAssociation::fromJson(const nlohmann::json& j) {
    VodAssociation v;
    v.matchUuid = j["matchUuid"].get<std::string>();
    v.userUuid = j["userUuid"].get<std::string>();
    v.videoUuid = j["videoUuid"].get<std::string>();
    v.startTime = shared::isoStrToTime(j["startTime"].get<std::string>());
    v.endTime = shared::isoStrToTime(j["endTime"].get<std::string>());
    v.rawContainerFormat = j["rawContainerFormat"].get<std::string>();
    v.isClip = j.value("isClip", false);
    v.isLocal = j.value("isLocal", false);
    return v;
}

nlohmann::json VodMetadata::toJson() const {
    nlohmann::json body = {
        { "videoUuid", videoUuid },
        { "resX", resX },
        { "resY", resY },
        { "minBitrate", minBitrate },
        { "avgBitrate", avgBitrate },
        { "maxBitrate", maxBitrate },
        { "bucket", bucket },
        { "id", id },
        { "fps", fps },
    };

    if (sessionId) {
        body["sessionId"] = sessionId.value();
    }

    return body;
}

std::optional<VodMetadata> generateVodMetadataFromFile(const std::filesystem::path& fname) {
    try {
        InputVideoFile file(fname);
        return file.metadata();
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to generate VOD metadata: " << ex.what() << std::endl);
        return std::nullopt;
    }
}

}