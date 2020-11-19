#pragma once

#include "process_watcher/process/process.h"
#include "shared/games.h"
#include "recorder/encoder/av_encoder.h"
#include "recorder/video/video_recorder.h"
#include "recorder/audio/portaudio_audio_recorder.h"
#include "recorder/pipe/file_output_piper.h"
#include "shared/squadov/vod.h"
#include <filesystem>
#include <memory>
#include <shared_mutex>
#include <thread>

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
    shared::squadov::VodMetadata getMetadata() const { 
        auto metadata = _encoder->getMetadata();
        metadata.videoUuid = _currentId->videoUuid;
        return metadata;
    }

private:
    void createVideoRecorder(const video::VideoWindowInfo& info);
    void updateWindowInfo();

    process_watcher::process::Process _process;
    std::filesystem::path _outputFolder;
    shared::EGame _game;

    encoder::AvEncoderPtr _encoder;
    std::unique_ptr<VodIdentifier> _currentId;
    pipe::FileOutputPiperPtr _outputPiper;

    video::VideoRecorderPtr _vrecorder;
    audio::AudioRecorderPtr _aoutRecorder;
    audio::AudioRecorderPtr _ainRecorder;

    bool _running = true;
    std::thread _updateWindowInfoThread;
    std::shared_mutex _windowInfoMutex;
    video::VideoWindowInfo _windowInfo;
};
using GameRecorderPtr = std::unique_ptr<GameRecorder>;

}