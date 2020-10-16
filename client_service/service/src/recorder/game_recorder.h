#pragma once

#include "process_watcher/process/process.h"
#include "shared/games.h"
#include "recorder/encoder/av_encoder.h"
#include "recorder/video/video_recorder.h"
#include "recorder/audio/portaudio_audio_recorder.h"
#include <filesystem>
#include <memory>

namespace service::recorder {

struct VodIdentifier {
    std::string userUuid;
    std::string videoUuid;
};

// Records a video of the specified game.
class GameRecorder {
public:
    GameRecorder(
        const process_watcher::process::Process& process,
        shared::EGame game
    );
    ~GameRecorder();

    VodIdentifier start();
    void stop();
    bool isRecording() const { return !!_encoder; }
    const VodIdentifier& currentId() const { return *_currentId; }

private:
    void createVideoRecorder();

    process_watcher::process::Process _process;
    std::filesystem::path _outputFolder;
    shared::EGame _game;

    encoder::AvEncoderPtr _encoder;
    std::unique_ptr<VodIdentifier> _currentId;

    video::VideoRecorderPtr _vrecorder;
    audio::AudioRecorderPtr _aoutRecorder;
    audio::AudioRecorderPtr _ainRecorder;
};
using GameRecorderPtr = std::unique_ptr<GameRecorder>;

}