#pragma once

#include "process_watcher/process/process.h"
#include "shared/games.h"
#include "recorder/encoder/av_encoder.h"
#include "recorder/video/video_recorder.h"
#include "recorder/audio/portaudio_audio_recorder.h"
#include "recorder/pipe/file_output_piper.h"
#include "shared/squadov/vod.h"
#include "shared/time.h"
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

    void start();
    void stop();

    VodIdentifier startFromSource(const std::filesystem::path& vodPath, const shared::TimePoint& vodStart, const shared::TimePoint& recordStart);
    void stopFromSource(const shared::TimePoint& end);

    bool isRecording() const { return !!_encoder || std::filesystem::exists(_manualVodPath); }
    const VodIdentifier& currentId() const { return *_currentId; }
    std::string sessionId() const { return _outputPiper->sessionId(); }

    shared::squadov::VodMetadata getMetadata() const;

private:
    void createVideoRecorder(const video::VideoWindowInfo& info);
    void updateWindowInfo();
    std::unique_ptr<VodIdentifier> createNewVodIdentifier() const;
    void initializeFileOutputPiper();

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

    // Manual Video File Controls
    std::filesystem::path _manualVodPath;
    shared::TimePoint _manualVodTimeStart;
    shared::TimePoint _manualVodRecordStart;
};
using GameRecorderPtr = std::unique_ptr<GameRecorder>;

}