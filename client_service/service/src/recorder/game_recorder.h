#pragma once

#include "process_watcher/process/process.h"
#include "shared/games.h"
#include "recorder/encoder/av_encoder.h"
#include "recorder/video/video_recorder.h"
#include "recorder/audio/portaudio_audio_recorder.h"
#include "recorder/pipe/file_output_piper.h"
#include "recorder/compositor/layers/compositor_layer.h"
#include "recorder/compositor/graph/fps_limiter_node.h"
#include "compositor/compositor.h"
#include "shared/squadov/vod.h"
#include "shared/time.h"
#include "system/settings.h"
#include "uploader/uploader.h"
#include <deque>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <vector>

namespace service::recorder {

struct VodIdentifier {
    std::string videoUuid;
};

struct GameRecordEnd {
    std::string matchUuid;
    shared::TimePoint endTime;
};

enum class RecordingMode {
    DVR,
    Normal
};

constexpr int FLAG_UNKNOWN = -1;
constexpr int FLAG_DXGI_RECORDING = 0b001;
constexpr int FLAG_GDI_RECORDING = 0b010;
constexpr int FLAG_WGC_RECORDING = 0b100;
constexpr int FLAG_ALL_RECORDING = FLAG_WGC_RECORDING | FLAG_GDI_RECORDING | FLAG_DXGI_RECORDING;

// Records a video of the specified game.
class GameRecorder {
public:
    GameRecorder(
        const process_watcher::process::Process& process,
        shared::EGame game
    );
    ~GameRecorder();

    // Starts the process of sending audio/video data to the encoder and generating encoded packets.
    // Those packets only get sent to an output if initializeDvrOutput and/or initializeVideoOutput
    // gets called.
    void start(int flags = FLAG_UNKNOWN);

    // This is a request to initialize and/or increase the DVR storage size in memory.
    void initializeDvrOutput(double sizeSeconds);
    void initializeVideoOutput(const shared::TimePoint& start, RecordingMode mode);

    void stop(std::optional<GameRecordEnd> end, bool keepLocal = false);
    void stopInputs();

    void forceUrl(const std::string& url) { _forcedOutputUrl = url; }
    void enableOverlay(bool enable);

    VodIdentifier startFromSource(const std::filesystem::path& vodPath, const shared::TimePoint& vodStart, const shared::TimePoint& recordStart);
    void stopFromSource(const shared::TimePoint& endTm, const GameRecordEnd& end);
    void setFileOutputFromDestination(const std::string& videoUuid, const service::uploader::UploadDestination& destination);

    bool isRecording() const { return !!_encoder.encoder || std::filesystem::exists(_manualVodPath); }
    const VodIdentifier& currentId() const { return *_currentId; }
    const shared::TimePoint& vodStartTime() const { return _vodStartTime; }
    std::string sessionId() const { return _outputPiper->sessionId(); }

    shared::squadov::VodMetadata getMetadata() const;
    void overrideResolution(size_t width, size_t height);
    void loadCachedInfo();
    void setNewProcess(const process_watcher::process::Process& process) { _process = process; }

    void addManualGameAudioProcess(const std::string& pName) { _auxGameProcessesForAudio.push_back(pName); }

private:
    struct EncoderDatum {
        encoder::AvEncoderPtr encoder;

        // Audio Input/Output -> Index into the appropriate vector of audio recorders -> encoder index to use.
        std::unordered_map<
            audio::EAudioDeviceDirection,
            std::unordered_map<size_t, size_t>
        > audioEncoderIndex;

        bool hasEncoder() const { return !!encoder; }
    };

    // Returns whether to allow an additional mouse recording
    bool createVideoRecorder(const video::VideoWindowInfo& info, int flags);
    void updateWindowInfo();
    std::unique_ptr<VodIdentifier> createNewVodIdentifier() const;
    void initializeFileOutputPiper();

    bool areInputStreamsInitialized() const;
    bool initializeInputStreams(int flags);
    bool initializeCompositor(const video::VideoWindowInfo& info, int flags);

    EncoderDatum createEncoder();

    process_watcher::process::Process _process;
    std::filesystem::path _outputFolder;
    shared::EGame _game;
    bool isGameEnabled() const;

    EncoderDatum _encoder;
    std::unique_ptr<VodIdentifier> _currentId;

    std::optional<service::uploader::UploadDestination> _outputDestination;
    pipe::FileOutputPiperPtr _outputPiper;
    std::optional<std::string> _forcedOutputUrl;
    shared::TimePoint _vodStartTime;
    service::recorder::encoder::AVSyncClock::time_point _syncClockTime;
    std::mutex _stopMutex;

    void switchToNewActiveEncoder(const EncoderDatum& data);
    service::recorder::compositor::CompositorPtr _compositor;
    video::VideoRecorderPtr _vrecorder;

    std::mutex _paInitMutex;
    audio::PortaudioInitRAIIPtr _paInit;

    std::vector<audio::AudioRecorderPtr> _aoutRecorder;
    std::vector<audio::AudioRecorderPtr> _ainRecorder;

    std::mutex _ainMutex;
    bool _streamsInit = false;

    std::optional<int64_t> _pttEnableCb = std::nullopt;
    std::optional<int64_t> _pttDisableCb = std::nullopt;
    std::optional<int64_t> _bookmarkCb = std::nullopt;
    std::optional<int64_t> _clipCb = std::nullopt;

    // Locked Settings. We used to need the video settings between the DVR recording
    // and the final video to stay the same so we cached the results here. Not sure how
    // necessary this is now.
    std::unique_ptr<video::VideoWindowInfo> _cachedWindowInfo;
    std::unique_ptr<service::system::RecordingSettings> _cachedRecordingSettings;
    int64_t _cachedInstantClipLengthSeconds = 15;    
    void clearCachedInfo();

    // Manual Video File Controls
    std::filesystem::path _manualVodPath;
    shared::TimePoint _manualVodTimeStart;
    shared::TimePoint _manualVodRecordStart;

    // Debug Variables if we ever feel like overriding the width and height.
    std::optional<size_t> _overrideWidth;
    std::optional<size_t> _overrideHeight;

    // Various layers and nodes of the compositor that we need control of.
    std::vector<service::recorder::compositor::layers::CompositorLayerPtr> _overlayLayers;
    service::recorder::compositor::graph::FpsLimiterNodePtr _fpsLimiter;

    // Certain games have other processes that spit out audio - capture that here.
    std::vector<std::string> _auxGameProcessesForAudio;
};
using GameRecorderPtr = std::unique_ptr<GameRecorder>;

}