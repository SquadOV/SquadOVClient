#pragma once

#include "process_watcher/process/process.h"
#include "shared/games.h"
#include "recorder/encoder/av_encoder.h"
#include "recorder/video/video_recorder.h"
#include "recorder/audio/portaudio_audio_recorder.h"
#include <filesystem>
#include <memory>

namespace service::recorder {

// Records a video of the specified game.
class GameRecorder {
public:
    GameRecorder(
        const process_watcher::process::Process& process,
        const std::filesystem::path& outputFolder,
        shared::EGame game
    );
    ~GameRecorder();

    std::filesystem::path start(const std::string& matchId);
    const std::filesystem::path& path() const { return _encoder->path(); }
    const std::filesystem::path& outputFolder() const { return _outputFolder; }
    void stop();
    bool isRecording() const { return !!_encoder; }

private:
    void createVideoRecorder();

    process_watcher::process::Process _process;
    std::filesystem::path _outputFolder;
    shared::EGame _game;

    encoder::AvEncoderPtr _encoder;
    video::VideoRecorderPtr _vrecorder;
    audio::AudioRecorderPtr _aoutRecorder;
    audio::AudioRecorderPtr _ainRecorder;
};
using GameRecorderPtr = std::unique_ptr<GameRecorder>;

}