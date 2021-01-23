#include "recorder/game_recorder.h"

#include "shared/base64/encode.h"
#include "shared/errors/error.h"
#include "shared/env.h"
#include "shared/time.h"
#include "shared/uuid.h"
#include "api/squadov_api.h"
#include "recorder/video/dxgi_desktop_recorder.h"
#include "recorder/video/win32_gdi_recorder.h"
#include "recorder/encoder/ffmpeg_av_encoder.h"
#include "recorder/audio/portaudio_audio_recorder.h"
#include "system/win32/hwnd_utils.h"
#include "system/state.h"
#include "system/settings.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

namespace service::recorder {

GameRecorder::GameRecorder(
    const process_watcher::process::Process& process,
    shared::EGame game):
    _process(process),
    _game(game) {

    // Start a new thread to determine the current window status of the game. We'll need it for initializing the
    // video recorder.
    _updateWindowInfoThread = std::thread(std::bind(&GameRecorder::updateWindowInfo, this));
}

GameRecorder::~GameRecorder() {
    if (isRecording()) {
        stop();
    }

    if (_updateWindowInfoThread.joinable()) {
        _running = false;
        _updateWindowInfoThread.join();
    }
}

void GameRecorder::updateWindowInfo() {
    if (_process.empty()) {
        return;
    }

    HWND wnd = service::system::win32::findWindowForProcessWithMaxDelay(_process.pid(), std::chrono::milliseconds(120000));
    while (_running) {
        if (!IsIconic(wnd)) {
            HMONITOR refMonitor = MonitorFromWindow(wnd, MONITOR_DEFAULTTOPRIMARY);
            RECT windowRes;
            GetClientRect(wnd, &windowRes);

            std::unique_lock<std::shared_mutex> guard(_windowInfoMutex);
            _windowInfo.width = windowRes.right - windowRes.left;
            _windowInfo.height = windowRes.bottom - windowRes.top;
            _windowInfo.init = true;
            _windowInfo.isWindowed = !service::system::win32::isFullscreen(wnd, refMonitor);
        }
        std::this_thread::sleep_for(100ms);
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

void GameRecorder::start(std::function<void()> cb) {
    if (!!_currentId) {
        return;
    }

    _currentId = createNewVodIdentifier();
    initializeFileOutputPiper();

    _encoder.reset(new encoder::FfmpegAvEncoder(_outputPiper->filePath()));

    // Just in case the user changed it in the UI already, just sync up via the file.
    service::system::getCurrentSettings()->reloadSettingsFromFile();
    const auto settings = service::system::getCurrentSettings()->recording();
    const auto windowInfo = [this](){
        std::shared_lock<std::shared_mutex> guard(_windowInfoMutex);
        return _windowInfo;
    }();
    
    // This needs to be in a separate thread so that we can wait for the window to be non-iconic before moving forward
    // in initialization.
    std::thread initThread([this, windowInfo, settings, cb](){
        try {
            createVideoRecorder(windowInfo);
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to create video recorder: " << ex.what() << std::endl);
            _encoder.reset(nullptr);
            _currentId = {};
            _outputPiper->stop();
            return;
        }

        // Initialize streams in the encoder here. Use hard-coded options for to record
        // video at 1080p@60fps.
        // TODO: Make some more quality controls for video/audio available here.
        // Note that we don't want to exceed 1080p for now (but we want to maintain the same aspect ratio as the OG resolution)
        const auto aspectRatio = static_cast<double>(_windowInfo.width) / _windowInfo.height;
        const auto desiredHeight = std::min(_windowInfo.height, static_cast<size_t>(settings.resY));
        const auto desiredWidth = static_cast<size_t>(desiredHeight * aspectRatio);
        LOG_INFO("Start Video Stream: " << desiredWidth << "x" << desiredHeight << " @" << settings.fps << "fps" << std::endl);
        _encoder->initializeVideoStream(
            settings.fps,
            desiredWidth,
            desiredHeight
        );

        // Create recorders to capture
        //   1) The video captured from the game.
        //   2) The audio from the user's system.
        //   3) The audio from the user's microphone (if any).
        _vrecorder->startRecording(_encoder.get(), static_cast<size_t>(settings.fps));

        _aoutRecorder.reset(new audio::PortaudioAudioRecorder(audio::EAudioDeviceDirection::Output));
        _ainRecorder.reset(new audio::PortaudioAudioRecorder(audio::EAudioDeviceDirection::Input));

        // For each audio stream we need to first start recording on the audio side to determine
        // the audio stream properties (e.g. # of samples, # of channels, sample rate) and then pass
        // that to the encoder so the encoder knows how to handle the audio packets it gets. The encoder
        // will then pass back information to let the audio recorder know which stream to record into.
        // The audio recorder will not pass audio packets to the encoder until it has that information.
        _encoder->initializeAudioStream();
        if (_aoutRecorder->exists()) {
            const auto encoderIndex = _encoder->addAudioInput(_aoutRecorder->props());
            _aoutRecorder->startRecording(_encoder.get(), encoderIndex);
        }

        if (_ainRecorder->exists()) {
            const auto encoderIndex = _encoder->addAudioInput(_ainRecorder->props());
            _ainRecorder->startRecording(_encoder.get(), encoderIndex);
        }

        _encoder->start();
        cb();
        system::getGlobalState()->markGameRecording(_game, true);
    });
    initThread.detach();
}

void GameRecorder::stop() {
    if (!_encoder) {
        return;
    }
    _vrecorder->stopRecording();
    _vrecorder.reset(nullptr);

    if (!!_aoutRecorder) {
        _aoutRecorder->stop();
        _aoutRecorder.reset(nullptr);
    }

    if (!!_ainRecorder) {
        _ainRecorder->stop();
        _ainRecorder.reset(nullptr);
    }
    _encoder->stop();
    _encoder.reset(nullptr);
    _currentId.reset(nullptr);
    _outputPiper->wait();
    system::getGlobalState()->markGameRecording(_game, false);
}

VodIdentifier GameRecorder::startFromSource(const std::filesystem::path& vodPath, const shared::TimePoint& vodStart, const shared::TimePoint& recordStart) {
    if (!!_currentId) {
        return *_currentId;
    }

    _currentId = createNewVodIdentifier();
    initializeFileOutputPiper();

    _manualVodPath = vodPath;
    _manualVodTimeStart = vodStart;
    _manualVodRecordStart = recordStart;

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
            << " -c:v h264_nvenc -c:a aac -movflags +empty_moov+frag_keyframe+default_base_moof -f mp4 "
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
}

std::unique_ptr<VodIdentifier> GameRecorder::createNewVodIdentifier() const {
    auto id = std::make_unique<VodIdentifier>();
    id->userUuid = service::api::getGlobalApi()->getSessionUserUuid();
    id->videoUuid = shared::generateUuidv4();
    return id;
}

void GameRecorder::initializeFileOutputPiper() {
    // Create a pipe to the destination file. Could be a Google Cloud Storage signed URL
    // or even a filesystem. The API will tell us the right place to pipe to - we'll need to
    // create an output piper of the appropriate type based on the given URI.
    const std::string outputUri = service::api::getGlobalApi()->createVodDestinationUri(_currentId->videoUuid);
    _outputPiper = pipe::createFileOutputPiper(_currentId->videoUuid, outputUri);
    _outputPiper->start();
}

shared::squadov::VodMetadata GameRecorder::getMetadata() const {
    if (_encoder) {
        auto metadata = _encoder->getMetadata();
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
