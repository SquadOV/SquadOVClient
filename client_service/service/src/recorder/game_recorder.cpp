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
#include "recorder/video/windows_graphics_capture.h"
#include "recorder/encoder/ffmpeg_av_encoder.h"
#include "recorder/audio/portaudio_audio_recorder.h"
#include "renderer/d3d11_renderer.h"
#include "system/win32/hwnd_utils.h"
#include "system/state.h"
#include "shared/math.h"
#include "shared/filesystem/local_record.h"
#include "vod/process.h"

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
        stop({});
    }
}

void GameRecorder::createVideoRecorder(const video::VideoWindowInfo& info, int flags) {
    LOG_INFO("Attempting to create video recorder: " << std::endl
        << "\tResolution: " << info.width << "x" << info.height << std::endl
        << "\tInit: " << info.init << std::endl
        << "\tWindowed: " << info.isWindowed << std::endl
        << "\tFlags: " << flags << std::endl);

#ifdef _WIN32
    auto* shared = service::renderer::getSharedD3d11Context();
    if (flags & FLAG_DXGI_RECORDING && video::tryInitializeDxgiDesktopRecorder(_vrecorder, info, _process.pid(), shared)) {
        return;
    }

    if (flags & FLAG_WGC_RECORDING && video::tryInitializeWindowsGraphicsCapture(_vrecorder, info, _process.pid(), shared)) {
        return;
    }

    if (flags & FLAG_GDI_RECORDING && video::tryInitializeWin32GdiRecorder(_vrecorder, info, _process.pid())) {
        return;
    }
#endif

    THROW_ERROR("Failed to create GameRecorder instance: " << shared::gameToString(_game));
}

void GameRecorder::switchToNewActiveEncoder(const EncoderDatum& data) {
    LOG_INFO("Switch inputs to point to new encoder..." << std::endl);
    if (!data.hasEncoder()) {
        LOG_INFO("...No encoder?" << std::endl);
        return;
    }

    LOG_INFO("\tSet video recorder encoder..." << std::endl);
    _vrecorder->setActiveEncoder(data.encoder.get());

    if (_ainRecorder && data.audioEncoderIndex.find(audio::EAudioDeviceDirection::Input) != data.audioEncoderIndex.end()) {
        LOG_INFO("\tSet audio input encoder..." << std::endl);
        _ainRecorder->setActiveEncoder(data.encoder.get(), data.audioEncoderIndex.at(audio::EAudioDeviceDirection::Input));
    }

    if (_aoutRecorder && data.audioEncoderIndex.find(audio::EAudioDeviceDirection::Output) != data.audioEncoderIndex.end()) {
        LOG_INFO("\tSet audio output encoder..." << std::endl);
        _aoutRecorder->setActiveEncoder(data.encoder.get(), data.audioEncoderIndex.at(audio::EAudioDeviceDirection::Output));
    }

    LOG_INFO("...Start new active encoder." << std::endl);
    data.encoder->start();
}

void GameRecorder::startNewDvrSegment() {
    if (!_dvrRunning) {
        return;
    }

    const auto dir = shared::filesystem::getSquadOvDvrSessionFolder() / fs::path(_dvrSessionId);
    startNewDvrSegment(dir);
}

void GameRecorder::startNewDvrSegment(const fs::path& dir) {
    std::ostringstream segmentFname;
    segmentFname << "segment_" << _dvrId++ << ".ts";
    LOG_INFO("Create New DVR Segment: " << segmentFname.str() << std::endl);

    // At the given interval, create a new DVR segment.
    DvrSegment segment;
    segment.outputPath = dir / fs::path(segmentFname.str());
    segment.startTime = shared::nowUtc();
    segment.endTime = shared::zeroTime();

    // Create a new DVR encoder. This encoder is responsible for outputting the video to the specified location on disk.
    EncoderDatum data = createEncoder(shared::filesystem::pathUtf8(segment.outputPath));

    // Switch the active encoder to this new encoder before flushing out the old encoder (if there is one).
    // We want this order to ensure that there's minimal loss of data between the two video files.
    LOG_INFO("\tSwitch inputs to new segment encoder..." << std::endl);
    switchToNewActiveEncoder(data);

    if (_dvrEncoder.hasEncoder()) {
        _dvrEncoder.encoder->stop();
    }

    _dvrEncoder = std::move(data);
    if (!_dvrSegments.empty()) {
        _dvrSegments.back().endTime = segment.startTime;
    }
    _dvrSegments.emplace_back(std::move(segment));

    if (_dvrSegments.size() > MAX_DVR_SEGMENTS) {
        _dvrSegments.front().cleanup();
        _dvrSegments.pop_front();
    }

    LOG_INFO("...Finishing creating DVR segment :: " << segmentFname.str() << std::endl);
}

void GameRecorder::startDvrSession(int flags, bool autoTick) {
    if (_encoder.hasEncoder()) {
        LOG_WARNING("Can not start DVR session while a VOD encoder is active." << std::endl);
        return;
    }

    loadCachedInfo();

    if (!areInputStreamsInitialized()) {
        std::future<bool> successFut = std::async(std::launch::async, &GameRecorder::initializeInputStreams, this, flags);
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

    _dvrThread = std::thread([this, threshold, step, dir, autoTick](){
        auto timeSinceLastDvrSegment = std::chrono::milliseconds(0);
        while (_dvrRunning) {
            if (autoTick && timeSinceLastDvrSegment >= threshold) {
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
    if (!_cachedRecordingSettings) {
        LOG_INFO("Load cache info: Settings" << std::endl);
        // Just in case the user changed it in the UI already, just sync up via the file.
        service::system::getCurrentSettings()->reloadSettingsFromFile();
        _cachedRecordingSettings = std::make_unique<service::system::RecordingSettings>(service::system::getCurrentSettings()->recording());
    }

    if (!_process.empty() && (!_cachedWindowInfo || !_cachedWindowInfo->init)) {
        LOG_INFO("Load cache info: Window Information" << std::endl);
        std::future<video::VideoWindowInfo> fut = std::async(std::launch::async, [this](){
            video::VideoWindowInfo ret;
            HWND wnd = service::system::win32::findWindowForProcessWithMaxDelay(_process.pid(), std::chrono::milliseconds(0));
            LOG_INFO("...Found Window! Detecting monitor and resolution..." << std::endl);
            if (wnd) {
                while (true) {
                    if (!IsIconic(wnd)) {
                        HMONITOR refMonitor = MonitorFromWindow(wnd, MONITOR_DEFAULTTOPRIMARY);
                        RECT windowRes;
                        if (!GetClientRect(wnd, &windowRes)) {
                            LOG_ERROR("Failed to get client rect: " << shared::errors::getWin32ErrorAsString() << std::endl);
                            break;
                        }

                        ret.width = windowRes.right - windowRes.left;
                        ret.height = windowRes.bottom - windowRes.top;
                        ret.init = true;
                        ret.isWindowed = !service::system::win32::isFullscreen(wnd, refMonitor, 4);
                        break;
                    }
                    std::this_thread::sleep_for(100ms);
                }
            }

            if (!ret.init) {
                LOG_WARNING("Failed to find window (final).");
            }

            return ret;
        });
        _cachedWindowInfo = std::make_unique<video::VideoWindowInfo>(fut.get());
    }
    LOG_INFO("Finish loading cache info." << std::endl);

    if (!_cachedRecordingSettings || (!_process.empty() && (!_cachedWindowInfo || !_cachedWindowInfo->init))) {
        THROW_ERROR("Failed to create cached info objects.");
    }
}

void GameRecorder::clearCachedInfo() {
    _cachedRecordingSettings.reset(nullptr);
    _cachedWindowInfo.reset(nullptr);
}

GameRecorder::EncoderDatum GameRecorder::createEncoder(const std::string& outputFname) {
    EncoderDatum data;
    LOG_INFO("Create FFmpeg Encoder" << std::endl);
    data.encoder = std::make_unique<encoder::FfmpegAvEncoder>(outputFname);

    const auto aspectRatio = static_cast<double>(_cachedWindowInfo->width) / _cachedWindowInfo->height;
    const auto desiredHeight = shared::math::forceEven(_overrideHeight.value_or(std::min(_cachedWindowInfo->height, static_cast<size_t>(_cachedRecordingSettings->resY))));
    const auto desiredWidth = shared::math::forceEven(_overrideWidth.value_or(static_cast<size_t>(desiredHeight * aspectRatio)));

    // Assume that the input recorders have already been created before this point.
    // This is primarily for the audio inputs so we know how many inputs to expect.
    LOG_INFO("Initialize video stream..." << std::endl);
    data.encoder->initializeVideoStream(
        *_cachedRecordingSettings,
        desiredWidth,
        desiredHeight
    );

    LOG_INFO("Initialize audio stream..." << std::endl);
    data.encoder->initializeAudioStream();
    if (_aoutRecorder && _aoutRecorder->exists()) {
        LOG_INFO("Adding audio output..." << std::endl);
        const auto encoderIndex = data.encoder->addAudioInput(_aoutRecorder->props());
        data.audioEncoderIndex[audio::EAudioDeviceDirection::Output] = encoderIndex;
    }

    if (_ainRecorder && _ainRecorder->exists()) {
        LOG_INFO("Adding audio input..." << std::endl);
        const auto encoderIndex = data.encoder->addAudioInput(_ainRecorder->props());
        data.audioEncoderIndex[audio::EAudioDeviceDirection::Input] = encoderIndex;
    }

    LOG_INFO("Open encoder..." << std::endl);
    data.encoder->open();
    return data;
}

bool GameRecorder::areInputStreamsInitialized() const {
    return _streamsInit;
}

bool GameRecorder::initializeInputStreams(int flags) {
    _streamsInit = true;
    try {
        createVideoRecorder(*_cachedWindowInfo, flags);
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to create video recorder: " << ex.what() << std::endl);
        _streamsInit = false;
        return false;
    }
    _vrecorder->startRecording();

    _aoutRecorder.reset(new audio::PortaudioAudioRecorder());
    _aoutRecorder->loadDevice(audio::EAudioDeviceDirection::Output, _cachedRecordingSettings->outputDevice, _cachedRecordingSettings->outputVolume);
    if (_aoutRecorder->exists()) {
        _aoutRecorder->startRecording();
    }

    _ainRecorder.reset(new audio::PortaudioAudioRecorder());
    _ainRecorder->loadDevice(audio::EAudioDeviceDirection::Input, _cachedRecordingSettings->inputDevice, _cachedRecordingSettings->inputVolume);
    if (_ainRecorder->exists()) {
        _ainRecorder->startRecording();
    }

    return true;
}

void GameRecorder::start(const shared::TimePoint& start, RecordingMode mode, int flags) {
    if (!!_currentId) {
        return;
    }

    LOG_INFO("Request VOD Record Start: " << shared::timeToStr(start) << std::endl);

    _currentId = createNewVodIdentifier();
    loadCachedInfo();

    try {
        initializeFileOutputPiper();
    } catch (std::exception& ex) {
        _currentId.reset(nullptr);
        LOG_ERROR("Failed to initialize output piper...ignoring start recording command: " << ex.what() << std::endl);
        return;
    }

    if (!areInputStreamsInitialized()) {
        LOG_INFO("Initialize input streams..." << std::endl);
        std::future<bool> successFut = std::async(std::launch::async, &GameRecorder::initializeInputStreams, this, flags);
        if (!successFut.get()) {
            LOG_WARNING("Failed to start game recording." << std::endl);
            _currentId = {};
            _outputPiper->stop();
            return;
        }
    }

    LOG_INFO("Creating encoder..." << std::endl);
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
        _outputPiper->pauseProcessingFromPipe(true);
    }
    
    // Needs to be after the pause or else we'll wait indefinitely for the pause lock when asking for a pause
    // as it'll wait on the named pipe.
    LOG_INFO("Start output piper..." << std::endl);
    _outputPiper->start();

    LOG_INFO("Switch to new active encoder..." << std::endl);
    switchToNewActiveEncoder(_encoder);

    if (useDvr) {
        LOG_INFO("Stop DVR session..." << std::endl);
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
        LOG_INFO("Choosing backfill index: " << startIndex + 1 << " out of " << _dvrSegments.size() << std::endl);
        for (auto i = startIndex; i < _dvrSegments.size(); ++i) {
            LOG_INFO("\tUse Backfill Video from [" << i << "]: " << shared::timeToStr(_dvrSegments[i].startTime) << " to " << shared::timeToStr(_dvrSegments[i].endTime) << std::endl);
            // We can directly append to the output pipe because of the fact that we're using MPEG-TS which is file-level concat-able.
            // This does result in a file that's not really default playable by most video players; however, we'll assume that the user
            // won't be able to view the video until it's processed by the server after which case the VOD should be normal.
            _outputPiper->appendFromFile(_dvrSegments[i].outputPath);
            totalBackFillTime += std::chrono::duration_cast<std::chrono::milliseconds>(_dvrSegments[i].endTime - _dvrSegments[i].startTime);
        }
        
        _outputPiper->pauseProcessingFromPipe(false);
        _vodStartTime = _dvrSegments[startIndex].startTime;
        if (totalBackFillTime > std::chrono::milliseconds(30 * 1000)) {
            LOG_WARNING("Backfilling more than 30 seconds of footage from DVR!!! [" << totalBackFillTime.count() / 1000.0 << "s]" << std::endl);
        }
        LOG_INFO("Finish DVR backfill." << std::endl);
        cleanDvrSession(sessionId);

        // This is a bit of a hack. For some reason sometimes the previous call to
        // switchToNewActiveEncoder will cause the DVR segment's switchToNewActiveEncoder
        // to become unstuck...which would then cause the segment's active encoder to be the
        // new "active encoder" while the actual active encoder is no longer receiving data.
        switchToNewActiveEncoder(_encoder);
    } else {
        _vodStartTime = shared::nowUtc();
    }

    LOG_INFO("Final VOD Start Time: " << shared::timeToStr(_vodStartTime) << std::endl);
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

void GameRecorder::stop(std::optional<GameRecordEnd> end, bool keepLocal) {
    LOG_INFO("Stop Game Recording...Clearing VOD ID" << std::endl);
    const auto vodId = _currentId ? currentId() : VodIdentifier{};
    const auto metadata = getMetadata();
    _currentId.reset(nullptr);

    LOG_INFO("Stop Inputs..." << std::endl);
    stopInputs();
    if (_dvrEncoder.hasEncoder()) {
        LOG_INFO("Stop DVR session..." << std::endl);
        const auto session = stopDvrSession();
        cleanDvrSession(session);
    }

    LOG_INFO("Retrieving local recording info..." << std::endl);
    const auto wasLocal = _cachedRecordingSettings->useLocalRecording;
    const auto singleton = shared::filesystem::LocalRecordingIndexDb::singleton();
    singleton->initializeFromFolder(fs::path(_cachedRecordingSettings->localRecordingLocation));

    LOG_INFO("Clear Cached Info..." << std::endl);
    clearCachedInfo();
    if (!isRecording()) {
        return;
    }

    const auto vodStartTime = this->vodStartTime();
    
    if (_encoder.hasEncoder()) {
        LOG_INFO("Stop primary encoder..." << std::endl);
        _encoder.encoder->stop();
        _encoder = {};
    }

    if (_outputPiper) {
        LOG_INFO("Stop output piper..." << std::endl);
        const auto sessionId = this->sessionId();

        // Move the output piper to a new thread to wait for it to finish
        // so we don't get bottlenecked by any user's poor internet speeds.
        // We only do VOD association when the upload ends so we don't tell the
        // server to associate a VOD that doesn't actually exist.
        std::thread uploadThread([this, vodId, metadata, sessionId, vodStartTime, end, wasLocal, singleton, keepLocal](){
            pipe::FileOutputPiperPtr outputPiper = std::move(_outputPiper);
            _outputPiper.reset(nullptr);
            outputPiper->wait();

            if (end.has_value()) {
                shared::squadov::VodAssociation association;
                association.matchUuid = end.value().matchUuid;
                association.userUuid = service::api::getGlobalApi()->getCurrentUser().uuid;
                association.videoUuid = vodId.videoUuid;
                association.startTime = vodStartTime;
                association.endTime = end.value().endTime;
                association.rawContainerFormat = wasLocal ? "mp4" : "mpegts";
                association.isLocal = wasLocal;

                try {
                    service::api::getGlobalApi()->associateVod(association, metadata, sessionId);
                    if (wasLocal && outputPiper->localFile().has_value()) {
                        const auto rawPath = outputPiper->localFile().value();
                        const auto processedPath = shared::filesystem::getSquadOvTempFolder() / fs::path(association.videoUuid) / fs::path("video.mp4");
                        service::vod::processRawLocalVod(rawPath, processedPath);

                        if (!fs::exists(processedPath)) {
                            THROW_ERROR("Failed find locally processed video at: " << processedPath << std::endl);
                        }

                        // We need to process this VOD locally (aka do the equivalent of the server side fastify).
                        shared::filesystem::LocalRecordingIndexEntry entry;
                        entry.uuid = association.videoUuid;
                        entry.filename = "video.mp4";
                        entry.cacheTm = shared::nowUtc();
                        entry.startTm = association.startTime;
                        entry.endTm = association.endTime;
                        entry.diskBytes = fs::file_size(processedPath);

                        // After that's complete, we can finally add this file to the local index.
                        singleton->addLocalEntryFromFilesystem(processedPath, entry);

                        const auto finalPathParentDir = singleton->getEntryPath(entry).parent_path();
                        const auto metadataFname = finalPathParentDir / fs::path("metadata.json");
                        std::ofstream metadataOut(metadataFname);
                        metadataOut << metadata.toJson().dump(4);
                    }
                } catch (std::exception& ex) {
                    LOG_WARNING("Failed to associate VOD: " << ex.what() << std::endl);
                    service::api::getGlobalApi()->deleteVod(vodId.videoUuid);
                }
            }

            if (wasLocal && outputPiper->localFile().has_value() && !keepLocal) {
                const auto pth = outputPiper->localFile().value();
                outputPiper.reset(nullptr);

                if (fs::exists(pth)) {
                    for (auto i = 0; i < 30; ++i) {
                        try {
                            fs::remove(pth);
                            break;
                        } catch (...) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(150));
                        }
                    }
                }
            }
        });
        uploadThread.detach();
    }
    system::getGlobalState()->markGameRecording(_game, false);
}

VodIdentifier GameRecorder::startFromSource(const std::filesystem::path& vodPath, const shared::TimePoint& vodStart, const shared::TimePoint& recordStart) {
    if (!!_currentId) {
        return *_currentId;
    }

    _currentId = createNewVodIdentifier();
    loadCachedInfo();
    initializeFileOutputPiper();
    _outputPiper->start();

    _manualVodPath = vodPath;
    _manualVodTimeStart = vodStart;
    _manualVodRecordStart = recordStart;
    _vodStartTime = recordStart;

    return *_currentId;
}

void GameRecorder::stopFromSource(const shared::TimePoint& endTm, const GameRecordEnd& end) {
    const auto vodId = currentId();
    const auto metadata = getMetadata();
    const auto sessionId = this->sessionId();
    const auto vodStartTime = this->vodStartTime();

    // We don't really expect this code to run in production so just do some janky
    // FFmpeg binary call here. YOLOOO.
    const auto ffmpegBinary = std::filesystem::path(shared::getEnv("FFMPEG_BINARY_PATH", ""));
    if (fs::exists(ffmpegBinary)) {
        const auto startMs = std::chrono::duration_cast<std::chrono::milliseconds>(_manualVodRecordStart - _manualVodTimeStart).count();
        const auto toMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTm - _manualVodTimeStart).count();

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

    shared::squadov::VodAssociation association;
    association.matchUuid = end.matchUuid;
    association.userUuid = service::api::getGlobalApi()->getCurrentUser().uuid;
    association.videoUuid = vodId.videoUuid;
    association.startTime = vodStartTime;
    association.endTime = end.endTime;
    association.rawContainerFormat = "mpegts";

    try {
        service::api::getGlobalApi()->associateVod(association, metadata, sessionId);
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to associate VOD: " << ex.what() << std::endl);
        service::api::getGlobalApi()->deleteVod(vodId.videoUuid);
    }
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
    const auto cloudUri = service::api::getGlobalApi()->createVodDestinationUri(_currentId->videoUuid, _cachedRecordingSettings->useLocalRecording ? "mp4" : "mpegts");
    const auto localTmpRecord = shared::filesystem::getSquadOvTempFolder()  / fs::path(_currentId->videoUuid) / fs::path("temp.ts");
    const std::string outputUri = _cachedRecordingSettings->useLocalRecording ? shared::filesystem::pathUtf8(localTmpRecord) : cloudUri;
    setFileOutputFromUri(_currentId->videoUuid, outputUri);
}

void GameRecorder::setFileOutputFromUri(const std::string& videoUuid, const std::string& uri) {
    LOG_INFO("Output URI: " << uri << std::endl);
    _outputPiper = pipe::createFileOutputPiper(videoUuid, uri);
    _outputPiper->setMaxUploadSpeed(_cachedRecordingSettings->maxUploadSpeed);
}

shared::squadov::VodMetadata GameRecorder::getMetadata() const {
    if (_encoder.encoder) {
        auto metadata = _encoder.encoder->getMetadata();
        metadata.videoUuid = _currentId->videoUuid;
        return metadata;
    } else {
        shared::squadov::VodMetadata metadata;
        metadata.id = "source";
        metadata.videoUuid = _currentId ? _currentId->videoUuid : "";
        
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

void GameRecorder::overrideResolution(size_t width, size_t height) {
    _overrideWidth = width;
    _overrideHeight = height;
}

}