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

    void startDvrSession(int flags = FLAG_UNKNOWN, bool autoTick = true);
    void startNewDvrSegment();
    void start(const shared::TimePoint& start, RecordingMode mode, int flags = FLAG_UNKNOWN);
    void stop(std::optional<GameRecordEnd> end, bool keepLocal = false);
    void stopInputs();
    std::string stopDvrSession();
    void cleanDvrSession(const std::string& id);
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

    EncoderDatum createEncoder(const std::string& outputFname);

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

    std::optional<int64_t> _pttEnableCb = 0;
    std::optional<int64_t> _pttDisableCb = 0;

    // Locked Settings. We need the video settings between the DVR recording
    // and the final video to stay the same so cache the results here.
    std::unique_ptr<video::VideoWindowInfo> _cachedWindowInfo;
    std::unique_ptr<service::system::RecordingSettings> _cachedRecordingSettings;
    void clearCachedInfo();

    // Manual Video File Controls
    std::filesystem::path _manualVodPath;
    shared::TimePoint _manualVodTimeStart;
    shared::TimePoint _manualVodRecordStart;

    // DVR Recording. Each time the user calls startDvrSession we create a new
    // DVR session UUID. This session UUID is able to give us a unique location
    // on the filesystem where we store the temporary videos. At a given interval
    // (e.g. 10 seconds), we create a new video file on disk by switching the recording
    // to a new encoder and flushing the old encoder to disk. When the user finally
    // wants to start recording for real starting at a certain time, we look through
    // all the DVR video file segments that we have stored and find the ones that
    // accurately represents the start time the user desires and prepend those to
    // the final encoder that'll send the video to GCS/VOD storage.
    std::string _dvrSessionId;
    bool _dvrRunning = false;
    std::thread _dvrThread;
    EncoderDatum _dvrEncoder;
    size_t _dvrId = 0;

    struct DvrSegment {
        std::filesystem::path outputPath;
        shared::TimePoint startTime;
        shared::TimePoint endTime;

        void cleanup() const;
    };
    std::deque<DvrSegment> _dvrSegments;
    void startNewDvrSegment(const std::filesystem::path& dir);
    size_t findDvrSegmentForVodStartTime(const shared::TimePoint& tm) const;

    // Debug Variables if we ever feel like overriding the width and height.
    std::optional<size_t> _overrideWidth;
    std::optional<size_t> _overrideHeight;

    // Various layers and nodes of the compositor that we need control of.
    std::vector<service::recorder::compositor::layers::CompositorLayerPtr> _overlayLayers;
    service::recorder::compositor::graph::FpsLimiterNodePtr _fpsLimiter;
};
using GameRecorderPtr = std::unique_ptr<GameRecorder>;

}