#include "recorder/game_recorder.h"


#include "shared/errors/error.h"
#include "shared/time.h"
#include "recorder/win32_gdi_recorder.h"
#include "recorder/video/ffmpeg_video_encoder.h"

#include <filesystem>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

namespace service::recorder {

GameRecorder::GameRecorder(
    const process_watcher::process::Process& process,
    const fs::path& outputFolder,
    shared::EGame game):
    _process(process),
    _outputFolder(outputFolder / fs::path(shared::gameToString(game))),
    _game(game) {
    if (!fs::exists(_outputFolder)) {
        fs::create_directories(_outputFolder);
    }
}

GameRecorder::~GameRecorder() {
    stop();
}

void GameRecorder::createInstance() {
#ifdef _WIN32
    if (tryInitializeWin32GdiRecorder(_instance, _process.pid())) {
        return;
    }
#endif

    THROW_ERROR("Failed to create GameRecorder instance: " << shared::gameToString(_game));
}

fs::path GameRecorder::start() {
    if (!!_instance && !!_encoder) {
        return _encoder->path();
    }

    // Generate an appropriate new filename for the video.
    std::ostringstream fname;
    fname << shared::fnameTimeToStr(shared::nowUtc()) << ".webm";

    const fs::path videoPath = _outputFolder / fs::path(fname.str());
    
    // Create an encoder to dump out the video to this path.
    _encoder.reset(new video::FfmpegVideoEncoder(videoPath));

    createInstance();
    _instance->startRecording(_encoder.get());
    return videoPath;
}

void GameRecorder::stop() {
    if (!_instance) {
        return;
    }
    _instance->stopRecording();
    _encoder.reset(nullptr);
    _instance.reset(nullptr);
}

}
