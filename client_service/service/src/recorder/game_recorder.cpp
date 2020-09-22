#include "recorder/game_recorder.h"


#include "shared/errors/error.h"
#include "shared/time.h"
#include "recorder/video/win32_gdi_recorder.h"
#include "recorder/encoder/ffmpeg_av_encoder.h"
#include "recorder/audio/portaudio_audio_recorder.h"

#include <filesystem>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

namespace service::recorder {

GameRecorder::GameRecorder(
    const process_watcher::process::Process& process,
    const fs::path& outputFolder,
    shared::EGame game):
    _process(process),
    _outputFolder(outputFolder / fs::path(shared::gameToString(game))),
    _game(game) {
    if (!fs::exists(_outputFolder)) {
        fs::create_directories(_outputFolder);
    }
}

GameRecorder::~GameRecorder() {
}

void GameRecorder::createVideoRecorder() {
#ifdef _WIN32
    if (video::tryInitializeWin32GdiRecorder(_vrecorder, _process.pid())) {
        return;
    }
#endif

    THROW_ERROR("Failed to create GameRecorder instance: " << shared::gameToString(_game));
}

std::filesystem::path GameRecorder::start(const std::string& matchId) {
    if (!!_encoder) {
        return _encoder->path();
    }

    // Generate an appropriate new basename for the video and audiot files.
    std::ostringstream fname;
    fname << matchId;
    const fs::path matchPath = _outputFolder / fs::path(fname.str());
    _encoder.reset(new encoder::FfmpegAvEncoder(matchPath));

    // Create recorders to capture
    //   1) The video captured from the game.
    //   2) The audio from the user's system.
    //   3) The audio from the user's microphone (if any).
    createVideoRecorder();
    _aoutRecorder.reset(new audio::PortaudioAudioRecorder(audio::EAudioDeviceDirection::Output));
    _ainRecorder.reset(new audio::PortaudioAudioRecorder(audio::EAudioDeviceDirection::Input));

    // Initialize streams in the encoder here. Use hard-coded options for to record
    // video at 1080p@60fps. 
    // TODO: Make some more quality controls for video/audio available here.
    _encoder->initializeVideoStream(60, 1920, 1080);
    _vrecorder->startRecording(_encoder.get());

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
    return _encoder->path();
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
}

}
