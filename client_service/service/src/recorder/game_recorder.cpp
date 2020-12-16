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

#include <chrono>
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

VodIdentifier GameRecorder::start() {
    if (!!_currentId) {
        return *_currentId;
    }

    _currentId = std::make_unique<VodIdentifier>();
    _currentId->userUuid = service::api::getGlobalApi()->getSessionUserUuid();
    _currentId->videoUuid = shared::generateUuidv4();

    // Create a pipe to the destination file. Could be a Google Cloud Storage signed URL
    // or even a filesystem. The API will tell us the right place to pipe to - we'll need to
    // create an output piper of the appropriate type based on the given URI.
    const std::string outputUri = service::api::getGlobalApi()->createVodDestinationUri(_currentId->videoUuid);
    _outputPiper = pipe::createFileOutputPiper(_currentId->videoUuid, outputUri);
    _outputPiper->start();
    _encoder.reset(new encoder::FfmpegAvEncoder(_outputPiper->filePath()));

    {
        std::shared_lock<std::shared_mutex> guard(_windowInfoMutex);

        // Initialize streams in the encoder here. Use hard-coded options for to record
        // video at 1080p@60fps. 
        // TODO: Make some more quality controls for video/audio available here.
        // Note that we don't want to exceed 1080p for now (but we want to maintain the same aspect ratio as the OG resolution)
        const auto aspectRatio = static_cast<double>(_windowInfo.width) / _windowInfo.height;
        const auto desiredHeight = std::min(_windowInfo.height, size_t(1080));
        _encoder->initializeVideoStream(
            60,
            static_cast<size_t>(desiredHeight * aspectRatio),
            desiredHeight
        );

        // Create recorders to capture
        //   1) The video captured from the game.
        //   2) The audio from the user's system.
        //   3) The audio from the user's microphone (if any).
        createVideoRecorder(_windowInfo);
        _vrecorder->startRecording(_encoder.get());
    }
    
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
    return *_currentId;
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
}

}
