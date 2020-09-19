#pragma once

#include "process_watcher/process/process.h"
#include "shared/games.h"
#include "recorder/video/video_encoder.h"
#include <filesystem>
#include <memory>

namespace service::recorder {

class RecorderInstance {
public:
    virtual ~RecorderInstance() {}
    virtual void startRecording(video::VideoEncoder* encoder) = 0;
    virtual void stopRecording() = 0;
    
protected:
    
};
using RecorderInstancePtr = std::unique_ptr<RecorderInstance>;

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
    void stop();

private:
    void createInstance();

    process_watcher::process::Process _process;
    std::filesystem::path _outputFolder;
    shared::EGame _game;

    RecorderInstancePtr _instance;
    video::VideoEncoderPtr _encoder;
};
using GameRecorderPtr = std::unique_ptr<GameRecorder>;

}