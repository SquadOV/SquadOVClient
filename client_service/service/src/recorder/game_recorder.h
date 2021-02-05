#pragma once

#include "process_watcher/process/process.h"
#include "shared/games.h"
#include "recorder/encoder/av_encoder.h"
#include "recorder/video/video_recorder.h"
#include "recorder/audio/portaudio_audio_recorder.h"
#include "recorder/pipe/file_output_piper.h"
#include "shared/squadov/vod.h"
#include "shared/time.h"
#include "system/settings.h"

#include <deque>
#include <filesystem>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <thread>
#include <unordered_map>

namespace service::recorder {

struct VodIdentifier {
    std::string videoUuid;
};

enum class RecordingMode {
    DVR,
    Normal
};

// Records a video of the specified game.
class GameRecorder {
public:
    GameRecorder(
        const process_watcher::process::Process& process,
        shared::EGame game
    );
    ~GameRecorder();

    void startDvrSession();
    void start(const shared::TimePoint& start, RecordingMode mode);
    void stop();
    void stopInputs();
    std::string stopDvrSession();
    void cleanDvrSession(const std::string& id);

    VodIdentifier startFromSource(const std::filesystem::path& vodPath, const shared::TimePoint& vodStart, const shared::TimePoint& recordStart);
    void stopFromSource(const shared::TimePoint& end);
    void setFileOutputFromUri(const std::string& videoUuid, const std::string& uri);

    bool isRecording() const { return !!_encoder.encoder || std::filesystem::exists(_manualVodPath); }
    const VodIdentifier& currentId() const { return *_currentId; }
    const shared::TimePoint& vodStartTime() const { return _vodStartTime; }
    std::string sessionId() const { return _outputPiper->sessionId(); }

    shared::squadov::VodMetadata getMetadata() const;

private:
    struct EncoderDatum {
        encoder::AvEncoderPtr encoder;
        std::unordered_map<audio::EAudioDeviceDirection, size_t> audioEncoderIndex;

        bool hasEncoder() const { return !!encoder; }
    };

    void createVideoRecorder(const video::VideoWindowInfo& info);
    void updateWindowInfo();
    std::unique_ptr<VodIdentifier> createNewVodIdentifier() const;
    void initializeFileOutputPiper();

    bool areInputStreamsInitialized() const;
    bool initializeInputStreams();
    EncoderDatum createEncoder(const std::string& outputFname);

    process_watcher::process::Process _process;
    std::filesystem::path _outputFolder;
    shared::EGame _game;

    EncoderDatum _encoder;
    std::unique_ptr<VodIdentifier> _currentId;
    pipe::FileOutputPiperPtr _outputPiper;
    shared::TimePoint _vodStartTime;

    void switchToNewActiveEncoder(const EncoderDatum& data);
    video::VideoRecorderPtr _vrecorder;
    audio::AudioRecorderPtr _aoutRecorder;
    audio::AudioRecorderPtr _ainRecorder;
    bool _streamsInit = false;

    // Locked Settings. We need the video settings between the DVR recording
    // and the final video to stay the same so cache the results here.
    std::unique_ptr<video::VideoWindowInfo> _cachedWindowInfo;
    std::unique_ptr<service::system::RecordingSettings> _cachedRecordingSettings;
    void loadCachedInfo();
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
};
using GameRecorderPtr = std::unique_ptr<GameRecorder>;

}