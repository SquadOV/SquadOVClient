#include "recorder/game_recorder.h"

#include "shared/base64/encode.h"
#include "shared/errors/error.h"
#include "shared/env.h"
#include "shared/time.h"
#include "shared/uuid.h"
#include "shared/filesystem/common_paths.h"
#include "shared/filesystem/utility.h"
#include "api/squadov_api.h"
#include "recorder/video/dxgi_desktop_recorder.h"
#include "recorder/video/win32_gdi_recorder.h"
#include "recorder/encoder/ffmpeg_av_encoder.h"
#include "recorder/audio/portaudio_audio_recorder.h"
#include "system/win32/hwnd_utils.h"
#include "system/state.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <future>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

namespace service::recorder {
namespace {

constexpr int DVR_SEGMENT_LENGTH_SECONDS = 30;
constexpr int MAX_DVR_SEGMENTS = 6;

}

void GameRecorder::DvrSegment::cleanup() const {
    // There's no particular reason the cleanup neesd to happen immediately so start off
    // a new thread to do it. We're not guaranteed that this will succeed immediately anyway
    // so this is for the best.
    fs::path copyOutputPath = outputPath;
    std::thread t([copyOutputPath](){
        if (fs::exists(copyOutputPath)) {
            for (auto i = 0; i < 30; ++i) {
                try {
                    fs::remove(copyOutputPath);
                    break;
                } catch (...) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(150));
                }
            }
        }
    });
    t.detach();
}

GameRecorder::GameRecorder(
    const process_watcher::process::Process& process,
    shared::EGame game):
    _process(process),
    _game(game) {
}

GameRecorder::~GameRecorder() {
    if (isRecording()) {
        stop();
    }
}

void GameRecorder::createVideoRecorder(const video::VideoWindowInfo& info) {
    LOG_INFO("Attempting to create video recorder: " << std::endl
        << "\tResolution: " << info.width << "x" << info.height << std::endl
        << "\tInit: " << info.init << std::endl
        << "\tWindowed" << info.isWindowed << std::endl);

#ifdef _WIN32
    if (video::tryInitializeDxgiDesktopRecorder(_vrecorder, info, _process.pid())) {
        return;
    }

    if (video::tryInitializeWin32GdiRecorder(_vrecorder, info, _process.pid())) {
        return;
    }
#endif

    THROW_ERROR("Failed to create GameRecorder instance: " << shared::gameToString(_game));
}

void GameRecorder::switchToNewActiveEncoder(const EncoderDatum& data) {
    if (!data.hasEncoder()) {
        return;
    }

    _vrecorder->setActiveEncoder(data.encoder.get());

    if (_ainRecorder && data.audioEncoderIndex.find(audio::EAudioDeviceDirection::Input) != data.audioEncoderIndex.end()) {
        _ainRecorder->setActiveEncoder(data.encoder.get(), data.audioEncoderIndex.at(audio::EAudioDeviceDirection::Input));
    }

    if (_aoutRecorder && data.audioEncoderIndex.find(audio::EAudioDeviceDirection::Output) != data.audioEncoderIndex.end()) {
        _aoutRecorder->setActiveEncoder(data.encoder.get(), data.audioEncoderIndex.at(audio::EAudioDeviceDirection::Output));
    }

    data.encoder->start();
}

void GameRecorder::startNewDvrSegment(const fs::path& dir) {
    std::ostringstream segmentFname;
    segmentFname << "segment_" << _dvrId++ << ".ts";

    // At the given interval, create a new DVR segment.
    DvrSegment segment;
    segment.outputPath = dir / fs::path(segmentFname.str());
    segment.startTime = shared::nowUtc();
    segment.endTime = shared::zeroTime();

    // Create a new DVR encoder. This encoder is responsible for outputting the video to the specified location on disk.
    EncoderDatum data = createEncoder(shared::filesystem::pathUtf8(segment.outputPath));

    // If we had an old encoder we want to transfer the old front buffer to be the new back buffer in the new encoder.
    // This way we remove the possibility of starting the new video with a green frame. This needs to happen before the
    // video encoding thread starts in switchToNewActiveEncoder.
    if (_dvrEncoder.hasEncoder()) {
        const auto frontBuffer = _dvrEncoder.encoder->getFrontBuffer();
        data.encoder->addVideoFrame(frontBuffer);
    }

    // Switch the active encoder to this new encoder before flushing out the old encoder (if there is one).
    // We want this order to ensure that there's minimal loss of data between the two video files.
    switchToNewActiveEncoder(data);

    if (_dvrEncoder.hasEncoder()) {
        _dvrEncoder.encoder->stop();
    }

    _dvrEncoder = std::move(data);
    if (_dvrSegments.size() > 1) {
        _dvrSegments.back().endTime = segment.startTime;
    }
    _dvrSegments.emplace_back(std::move(segment));

    if (_dvrSegments.size() > MAX_DVR_SEGMENTS) {
        _dvrSegments.front().cleanup();
        _dvrSegments.pop_front();
    }
}

void GameRecorder::startDvrSession() {
    if (_encoder.hasEncoder()) {
        LOG_WARNING("Can not start DVR session while a VOD encoder is active." << std::endl);
        return;
    }

    loadCachedInfo();

    if (!areInputStreamsInitialized()) {
        std::future<bool> successFut = std::async(std::launch::async, &GameRecorder::initializeInputStreams, this);
        if (!successFut.get()) {
            LOG_WARNING("Failed to start DVR session." << std::endl);
            return;
        }
    }

    _dvrSessionId = shared::generateUuidv4();
    _dvrRunning = true;
    _dvrId = 0;

    // Do initial setup here so that we don't return from this function until we actually start DVR recording.
    const auto threshold = std::chrono::milliseconds(DVR_SEGMENT_LENGTH_SECONDS * 1000);
    const auto step = std::chrono::milliseconds(100);

    const auto dir = shared::filesystem::getSquadOvDvrSessionFolder() / fs::path(_dvrSessionId);
    if (!fs::exists(dir)) {
        fs::create_directories(dir);
    }
    startNewDvrSegment(dir);

    _dvrThread = std::thread([this, threshold, step, dir](){
        auto timeSinceLastDvrSegment = std::chrono::milliseconds(0);
        while (_dvrRunning) {
            if (timeSinceLastDvrSegment >= threshold) {
                timeSinceLastDvrSegment = std::chrono::milliseconds(0);
                startNewDvrSegment(dir);
            }

            std::this_thread::sleep_for(step);
            timeSinceLastDvrSegment += step;
        }

        if (_dvrEncoder.hasEncoder()) {
            _dvrEncoder.encoder->stop();
        }
    });
}

std::string GameRecorder::stopDvrSession() {
    _dvrRunning = false;
    if (_dvrThread.joinable()) {
        _dvrThread.join();
        _dvrThread = {};
    }

    if (!_dvrSegments.empty()) {
        _dvrSegments.back().endTime = shared::nowUtc();
    }

    const auto id = _dvrSessionId;
    _dvrSessionId.clear();
    return id;
}

void GameRecorder::cleanDvrSession(const std::string& id) {
    if (id.empty()) {
        return;
    }

    const auto dir = shared::filesystem::getSquadOvDvrSessionFolder() / fs::path(_dvrSessionId);
    _dvrSegments.clear();

    std::thread t([dir](){
        if (!fs::exists(dir)) {
            return;
        }
        for (int i = 0; i < 30; ++i) {
            try {
                fs::remove_all(dir);
                break;
            } catch(...) {
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }
        }
    });
    t.detach();
}

void GameRecorder::loadCachedInfo() {
    if (_process.empty()) {
        LOG_ERROR("Failed to load cached info from empty process." << std::endl);
        return;
    }

    if (_cachedRecordingSettings || _cachedWindowInfo) {
        return;
    }

    LOG_INFO("Load cache info: Settings" << std::endl);
    // Just in case the user changed it in the UI already, just sync up via the file.
    service::system::getCurrentSettings()->reloadSettingsFromFile();
    _cachedRecordingSettings = std::make_unique<service::system::RecordingSettings>(service::system::getCurrentSettings()->recording());

    LOG_INFO("Load cache info: Window Information" << std::endl);
    std::future<video::VideoWindowInfo> fut = std::async(std::launch::async, [this](){
        video::VideoWindowInfo ret;
        HWND wnd = service::system::win32::findWindowForProcessWithMaxDelay(_process.pid(), std::chrono::milliseconds(120000));
        while (true) {
            if (!IsIconic(wnd)) {
                HMONITOR refMonitor = MonitorFromWindow(wnd, MONITOR_DEFAULTTOPRIMARY);
                RECT windowRes;
                GetClientRect(wnd, &windowRes);

                ret.width = windowRes.right - windowRes.left;
                ret.height = windowRes.bottom - windowRes.top;
                ret.init = true;
                ret.isWindowed = !service::system::win32::isFullscreen(wnd, refMonitor);
                break;
            }
            std::this_thread::sleep_for(100ms);
        }
        return ret;
    });
    _cachedWindowInfo = std::make_unique<video::VideoWindowInfo>(fut.get());
    LOG_INFO("Finish loading cache info." << std::endl);

    if (!_cachedRecordingSettings || !_cachedWindowInfo) {
        THROW_ERROR("Failed to create cached info objects.");
    }
}

void GameRecorder::clearCachedInfo() {
    _cachedRecordingSettings.reset(nullptr);
    _cachedWindowInfo.reset(nullptr);
}

GameRecorder::EncoderDatum GameRecorder::createEncoder(const std::string& outputFname) {
    EncoderDatum data;
    data.encoder = std::make_unique<encoder::FfmpegAvEncoder>(outputFname);

    const auto aspectRatio = static_cast<double>(_cachedWindowInfo->width) / _cachedWindowInfo->height;
    const auto desiredHeight = std::min(_cachedWindowInfo->height, static_cast<size_t>(_cachedRecordingSettings->resY));
    const auto desiredWidth = static_cast<size_t>(desiredHeight * aspectRatio);

    // Assume that the input recorders have already been created before this point.
    // This is primarily for the audio inputs so we know how many inputs to expect.
    data.encoder->initializeVideoStream(
        _cachedRecordingSettings->fps,
        desiredWidth,
        desiredHeight
    );

    data.encoder->initializeAudioStream();
    if (_aoutRecorder && _aoutRecorder->exists()) {
        const auto encoderIndex = data.encoder->addAudioInput(_aoutRecorder->props());
        data.audioEncoderIndex[audio::EAudioDeviceDirection::Output] = encoderIndex;
    }

    if (_ainRecorder && _ainRecorder->exists()) {
        const auto encoderIndex = data.encoder->addAudioInput(_ainRecorder->props());
        data.audioEncoderIndex[audio::EAudioDeviceDirection::Input] = encoderIndex;
    }

    data.encoder->open();
    return data;
}

bool GameRecorder::areInputStreamsInitialized() const {
    return _streamsInit;
}

bool GameRecorder::initializeInputStreams() {
    _streamsInit = true;
    try {
        createVideoRecorder(*_cachedWindowInfo);
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to create video recorder: " << ex.what() << std::endl);
        _streamsInit = false;
        return false;
    }
    _vrecorder->startRecording(_cachedRecordingSettings->fps);

    _aoutRecorder.reset(new audio::PortaudioAudioRecorder(audio::EAudioDeviceDirection::Output));
    if (_aoutRecorder->exists()) {
        _aoutRecorder->startRecording();
    }

    _ainRecorder.reset(new audio::PortaudioAudioRecorder(audio::EAudioDeviceDirection::Input));
    if (_ainRecorder->exists()) {
        _ainRecorder->startRecording();
    }

    return true;
}

void GameRecorder::start(const shared::TimePoint& start, RecordingMode mode) {
    if (!!_currentId) {
        return;
    }

    loadCachedInfo();
    _currentId = createNewVodIdentifier();
    initializeFileOutputPiper();

    if (!areInputStreamsInitialized()) {
        std::future<bool> successFut = std::async(std::launch::async, &GameRecorder::initializeInputStreams, this);
        if (!successFut.get()) {
            LOG_WARNING("Failed to start game recording." << std::endl);
            _currentId = {};
            _outputPiper->stop();
            return;
        }
    }

    _encoder = createEncoder(_outputPiper->filePath());

    // So there's two scenarios we can encounter here: 1) a DVR session is active AND we want to use it
    // or 2) no DVR session is active OR we don't particularly care for it. In the case where we want and can
    // use DVR, we use the input start time to look through our existing DVR recordings to find the segments
    // that best represent the start of the video. We also need to tell the encoder to hold off on encoding
    // the data it gets from the recorders until the DVR processing is finished.
    const auto useDvr = (mode == RecordingMode::DVR && _dvrRunning);

    // Note that we must pause the input recorder processing beforing switching the inputs to use the new encoder.
    // Otherwise the first frames may be of what's currently recording instead of what's recorded in the DVR.
    if (useDvr) {
        if (_dvrEncoder.hasEncoder()) {
            const auto frontBuffer = _dvrEncoder.encoder->getFrontBuffer();
            _encoder.encoder->addVideoFrame(frontBuffer);
        }
        _outputPiper->pauseProcessingFromPipe(true);
    }
    // Needs to be after the pause or else we'll wait indefinitely for the pause lock when asking for a pause
    // as it'll wait on the named pipe.
    _outputPiper->start();

    switchToNewActiveEncoder(_encoder);

    if (useDvr) {
        // We can stop this DVR session at this point. It'll be up to the caller to re-start another DVR
        // session when/if this recording ends.
        const auto sessionId = stopDvrSession();
        
        // This should be here because we don't want to destruct it too soon or else we won't have switched to the new
        // active encoder and we'll thus risk sending info to an invalid pointer.
        _dvrEncoder = {};
        LOG_INFO("DVR Backfill Session: " << sessionId << std::endl);

        // Now that we know all the recorder data is being buffered, we can safely do DVR processing. Loop through
        // the segments to find the segment with a start time before the input start time. If we can't, do a best effort.
        // At this point we know that the 1) DVR is no longer running and 2) all new video/audio packets are being buffered
        // in the new encoder.
        std::chrono::milliseconds totalBackFillTime(0);
        const auto startIndex = findDvrSegmentForVodStartTime(start);
        for (auto i = startIndex; i < _dvrSegments.size(); ++i) {
            // We can directly append to the output pipe because of the fact that we're using MPEG-TS which is file-level concat-able.
            // This does result in a file that's not really default playable by most video players; however, we'll assume that the user
            // won't be able to view the video until it's processed by the server after which case the VOD should be normal.
            _outputPiper->appendFromFile(_dvrSegments[i].outputPath);
            totalBackFillTime += std::chrono::duration_cast<std::chrono::milliseconds>(_dvrSegments[i].endTime - _dvrSegments[i].startTime);
        }
        
        _outputPiper->pauseProcessingFromPipe(false);
        _vodStartTime = (_dvrSegments[startIndex].startTime < start) ? start : _dvrSegments[startIndex].startTime;
        if (totalBackFillTime > std::chrono::milliseconds(30 * 1000)) {
            LOG_WARNING("Backfilling more than 30 seconds of footage from DVR!!!" << std::endl);
        }
        LOG_INFO("Finish DVR backfill." << std::endl);
        cleanDvrSession(sessionId);
    } else {
        _vodStartTime = shared::nowUtc();
    }

    system::getGlobalState()->markGameRecording(_game, true);
}

size_t GameRecorder::findDvrSegmentForVodStartTime(const shared::TimePoint& tm) const {
    for (auto i = 0; i < _dvrSegments.size(); ++i) {
        if (_dvrSegments[i].startTime <= tm && (_dvrSegments[i].endTime == shared::zeroTime() || _dvrSegments[i].endTime >= tm)) {
            return i;
        }
    }
    return _dvrSegments.size() - 1;
}

void GameRecorder::stopInputs() {
    _streamsInit = false;
    if (_vrecorder) {
        _vrecorder->stopRecording();
        _vrecorder.reset(nullptr);
    }

    if (_aoutRecorder) {
        _aoutRecorder->stop();
        _aoutRecorder.reset(nullptr);
    }

    if (_ainRecorder) {
        _ainRecorder->stop();
        _ainRecorder.reset(nullptr);
    }
}

void GameRecorder::stop() {
    stopInputs();
    if (_dvrEncoder.hasEncoder()) {
        const auto session = stopDvrSession();
        cleanDvrSession(session);
    }
    clearCachedInfo();
    if (_encoder.hasEncoder()) {
        _encoder.encoder->stop();
        _encoder = {};
    }
    _currentId.reset(nullptr);
    if (_outputPiper) {
        _outputPiper->wait();
        _outputPiper.reset(nullptr);
    }
    system::getGlobalState()->markGameRecording(_game, false);
}

VodIdentifier GameRecorder::startFromSource(const std::filesystem::path& vodPath, const shared::TimePoint& vodStart, const shared::TimePoint& recordStart) {
    if (!!_currentId) {
        return *_currentId;
    }

    _currentId = createNewVodIdentifier();
    initializeFileOutputPiper();
    _outputPiper->start();

    _manualVodPath = vodPath;
    _manualVodTimeStart = vodStart;
    _manualVodRecordStart = recordStart;
    _vodStartTime = recordStart;

    return *_currentId;
}

void GameRecorder::stopFromSource(const shared::TimePoint& end) {
    // We don't really expect this code to run in production so just do some janky
    // FFmpeg binary call here. YOLOOO.
    const auto ffmpegBinary = std::filesystem::path(shared::getEnv("FFMPEG_BINARY_PATH", ""));
    if (fs::exists(ffmpegBinary)) {
        const auto startMs = std::chrono::duration_cast<std::chrono::milliseconds>(_manualVodRecordStart - _manualVodTimeStart).count();
        const auto toMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - _manualVodTimeStart).count();

        std::ostringstream ffmpegCommand;
        ffmpegCommand << "\"\"" << ffmpegBinary.string() << "\""
            << " -y -i \"" << _manualVodPath.string() << "\""
            << " -ss " << static_cast<double>(startMs / 1000.0)
            << " -to " << static_cast<double>(toMs / 1000.0)
            << " -c:v h264_nvenc -c:a aac -f mpegts "
            << _outputPiper->filePath()
            << "\"";

        LOG_INFO("Run FFmpeg Command on Manual VOD source: " << ffmpegCommand.str() << std::endl);
        if (std::system(ffmpegCommand.str().c_str())) {
            LOG_ERROR("Failed to run FFmpeg." << std::endl);
        }
    } else {
        LOG_ERROR("Failed to find FFmpeg binary:" << ffmpegBinary.string() << std::endl);
    }

    _manualVodPath = std::filesystem::path();
    _manualVodTimeStart = shared::zeroTime();
    _currentId.reset(nullptr);
    _outputPiper->wait();
    _outputPiper.reset(nullptr);
}

std::unique_ptr<VodIdentifier> GameRecorder::createNewVodIdentifier() const {
    auto id = std::make_unique<VodIdentifier>();
    id->videoUuid = shared::generateUuidv4();
    return id;
}

void GameRecorder::initializeFileOutputPiper() {
    if (_outputPiper) {
        return;
    }

    // Create a pipe to the destination file. Could be a Google Cloud Storage signed URL
    // or even a filesystem. The API will tell us the right place to pipe to - we'll need to
    // create an output piper of the appropriate type based on the given URI.
    const std::string outputUri = service::api::getGlobalApi()->createVodDestinationUri(_currentId->videoUuid, "mpegts");
    setFileOutputFromUri(_currentId->videoUuid, outputUri);
}

void GameRecorder::setFileOutputFromUri(const std::string& videoUuid, const std::string& uri) {
    _outputPiper = pipe::createFileOutputPiper(videoUuid, uri);
}

shared::squadov::VodMetadata GameRecorder::getMetadata() const {
    if (_encoder.encoder) {
        auto metadata = _encoder.encoder->getMetadata();
        metadata.videoUuid = _currentId->videoUuid;
        return metadata;
    } else {
        shared::squadov::VodMetadata metadata;
        metadata.id = "source";
        metadata.videoUuid = _currentId->videoUuid;
        
        // TODO: Need to read in data from the file to get this information properly
        // instead of just randomly hard-coded numbers.
        metadata.resX = 1280;
        metadata.resY = 720;
        metadata.avgBitrate = 2011000;
        metadata.minBitrate = 2011000;
        metadata.maxBitrate = 2011000;
        metadata.fps = 30;

        return metadata;
    }
}

}