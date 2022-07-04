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
#include "recorder/audio/wasapi_audio_recorder.h"
#include "recorder/audio/wasapi_program_recorder.h"
#include "recorder/compositor/graph/texture_context_normalizer_node.h"
#include "recorder/compositor/graph/sink_node.h"
#include "recorder/compositor/layers/mouse_cursor_layer.h"
#include "recorder/compositor/layers/overlay_layers.h"
#include "recorder/compositor/layers/watermark_layer.h"
#include "renderer/d3d11_renderer.h"
#include "shared/system/win32/hwnd_utils.h"
#include "system/state.h"
#include "system/win32/message_loop.h"
#include "shared/math.h"
#include "shared/filesystem/local_record.h"
#include "shared/system/win32/interfaces/win32_system_process_interface.h"
#include "shared/strings/strings.h"
#include "system/notification_hub.h"
#include "vod/process.h"
#include "recorder/default_flags.h"
#include "process_watcher/process/process.h"

#include <boost/functional/hash.hpp>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <future>
#include <iostream>
#include <sstream>
#include <portaudio.h>
#include <unordered_set>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

namespace service::recorder {

GameRecorder::GameRecorder(
    const process_watcher::process::Process& process,
    shared::EGame game
):
    _process(process),
    _game(game)
{

    if (!isGameEnabled()) {
        DISPLAY_NOTIFICATION(
            service::system::NotificationSeverity::Error,
            service::system::NotificationDisplayType::NativeNotification,
            "SquadOV :: Disabled Game Recording",
            "You have disabled " << shared::gameToString(_game) << " recording. No VODs will be recorded."
        );
    }

    if (service::system::getGlobalState()->isPaused()) {
        DISPLAY_NOTIFICATION(
            service::system::NotificationSeverity::Error,
            service::system::NotificationDisplayType::NativeNotification,
            "SquadOV :: Paused Game Recording",
            "You have recording paused. No VODs will be recorded until you turn it back on."
        );
    }
}

GameRecorder::~GameRecorder() {
    if (isRecording()) {
        stop({});
    }
}

bool GameRecorder::createVideoRecorder(const video::VideoWindowInfo& info, int flags) {
    LOG_INFO("Attempting to create video recorder: " << std::endl
        << "\tResolution: " << info.width << "x" << info.height << std::endl
        << "\tInit: " << info.init << std::endl
        << "\tWindowed: " << info.isWindowed << std::endl
        << "\tFlags: " << flags << std::endl);

#ifdef _WIN32
    LOG_INFO("Trying DXGI..." << std::endl);
    if (flags & FLAG_DXGI_RECORDING && video::tryInitializeDxgiDesktopRecorder(_vrecorder, info, _process.pid())) {
        return true;
    }

    LOG_INFO("Trying GDI..." << std::endl);
    if (flags & FLAG_GDI_RECORDING && video::tryInitializeWin32GdiRecorder(_vrecorder, info, _process.pid())) {
        return true;
    }

    LOG_INFO("Trying WGC..." << std::endl);
    if (flags & FLAG_WGC_RECORDING && video::tryInitializeWindowsGraphicsCapture(_vrecorder, info, _process.pid())) {
        return false;
    }
#endif

    THROW_ERROR("Failed to create GameRecorder instance: " << shared::gameToString(_game));
    return true;
}

void GameRecorder::switchToNewActiveEncoder(const EncoderDatum& data) {
    LOG_INFO("Switch inputs to point to new encoder..." << std::endl);
    if (!data.hasEncoder()) {
        LOG_INFO("...No encoder?" << std::endl);
        return;
    }

    LOG_INFO("\tSet compositor encoder..." << std::endl);
    assert(_compositor);
    _compositor->setActiveEncoder(data.encoder.get());

    {
        std::lock_guard guard(_ainMutex);
        LOG_INFO("\tSet audio input encoder..." << std::endl);
        for (size_t i = 0; i < _ainRecorder.size(); ++i) {
            const auto inputIt = data.audioEncoderIndex.find(audio::EAudioDeviceDirection::Input);
            if (inputIt == data.audioEncoderIndex.end()) {
                continue;
            }

            const auto indexIt = inputIt->second.find(i);
            if (indexIt == inputIt->second.end()) {
                continue;
            }
            
            _ainRecorder[i]->setActiveEncoder(data.encoder.get(), indexIt->second);
        }
    }

    LOG_INFO("\tSet audio output encoder..." << std::endl);
    for (size_t i = 0; i < _aoutRecorder.size(); ++i) {
        const auto outputIt = data.audioEncoderIndex.find(audio::EAudioDeviceDirection::Output);
        if (outputIt == data.audioEncoderIndex.end()) {
            continue;
        }

        const auto indexIt = outputIt->second.find(i);
        if (indexIt == outputIt->second.end()) {
            continue;
        }

        _aoutRecorder[i]->setActiveEncoder(data.encoder.get(), indexIt->second);
    }

    LOG_INFO("...Start new active encoder." << std::endl);
    data.encoder->start();

    if (_fpsLimiter) {
        _fpsLimiter->setStartFrameTime(data.encoder->getSyncStartTime());
    }
}

void GameRecorder::loadCachedInfo() {
    // Just in case the user changed settings in the UI already, just sync up via the file.
    service::system::getCurrentSettings()->reloadSettingsFromFile();

    _cachedInstantClipLengthSeconds = service::system::getCurrentSettings()->instantClipLengthSeconds();
    if (!_cachedRecordingSettings) {
        LOG_INFO("Load cache info: Settings" << std::endl);
        _cachedRecordingSettings = std::make_unique<service::system::RecordingSettings>(service::system::getCurrentSettings()->recording());

        // There's some things that need to be limited by what we've determined the user's limitations to be on the server side.
        try {
            service::api::getGlobalApi()->retrieveSessionFeatureFlags();
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to update session feature flags: " << ex.what() << std::endl << "\tUsing Previously Stored Feature Flags." << std::endl);
        }

        const auto features = service::api::getGlobalApi()->getSessionFeatures();
        _cachedRecordingSettings->resY = std::min(_cachedRecordingSettings->resY, features.maxRecordPixelY);
        _cachedRecordingSettings->fps = std::min(_cachedRecordingSettings->fps, features.maxRecordFps);
        _cachedRecordingSettings->bitrateKbps = std::min(_cachedRecordingSettings->bitrateKbps, features.maxBitrateKbps);
        _cachedRecordingSettings->useLocalRecording = _cachedRecordingSettings->useLocalRecording || !features.allowRecordUpload;
        _cachedRecordingSettings->watermark.enabled |= features.mandatoryWatermark;
        _cachedRecordingSettings->watermark.size = std::max(features.watermarkMinSize, _cachedRecordingSettings->watermark.size);
        _cachedRecordingSettings->useSeparateAudioChannels = _cachedRecordingSettings->useSeparateAudioChannels && features.allowSeparateAudioChannels;

        if (_cachedRecordingSettings->useLocalRecording) {
#ifdef _WIN32
            ULARGE_INTEGER freeBytesForCaller = {0};
            ULARGE_INTEGER totalBytes = {0};
            ULARGE_INTEGER freeBytes = {0};
            if (GetDiskFreeSpaceExW(
                _cachedRecordingSettings->localRecordingLocation.native().c_str(),
                &freeBytesForCaller,
                &totalBytes,
                &freeBytes)) {
                LOG_INFO("Performing Local Recording: " << _cachedRecordingSettings->localRecordingLocation << std::endl
                    << "\tFree Bytes For Caller (GB): " << freeBytesForCaller.QuadPart / 1024.0 / 1024.0 / 1024.0 << std::endl
                    << "\tTotal Bytes (GB): " << totalBytes.QuadPart / 1024.0 / 1024.0 / 1024.0 << std::endl
                    << "\tFree Bytes (GB): " << freeBytes.QuadPart / 1024.0 / 1024.0 / 1024.0 << std::endl
                );

                if ((freeBytesForCaller.QuadPart  / 1024.0 / 1024.0 / 1024.0) <= 10.0) {
                    LOG_WARNING("!!! LOW DISK SPACE FOR LOCAL RECORDING !!!" << std::endl);
                    DISPLAY_NOTIFICATION(
                        service::system::NotificationSeverity::Error,
                        service::system::NotificationDisplayType::NativeNotification,
                        "SquadOV :: Low Disk Space",
                        "You have local recording turned on and are running out of space on your hard drive. Your VODs may not be saved!"
                    );
                }
            } else {
                LOG_WARNING("Failed to measure disk space on user machine: " << shared::errors::getWin32ErrorAsString() << std::endl);
            }
#endif
        }
    }

    if (!_process.empty() && (!_cachedWindowInfo || !_cachedWindowInfo->init)) {
        LOG_INFO("Load cache info: Window Information" << std::endl);
        std::future<video::VideoWindowInfo> fut = std::async(std::launch::async, [this](){
            video::VideoWindowInfo ret;
            HWND wnd = shared::system::win32::findWindowForProcessWithMaxDelay(_process.pid(), std::chrono::milliseconds(0));
            LOG_INFO("...Found Window! Detecting monitor and resolution..." << std::endl);
            if (wnd) {
                ret.window = wnd;
                while (true) {
                    if (!IsIconic(wnd)) {
                        HMONITOR refMonitor = MonitorFromWindow(wnd, MONITOR_DEFAULTTOPRIMARY);
                        RECT windowRes;
                        if (!GetClientRect(wnd, &windowRes)) {
                            LOG_ERROR("Failed to get client rect: " << shared::errors::getWin32ErrorAsString() << std::endl);
                            break;
                        }

                        ret.monitor = refMonitor;
                        ret.width = windowRes.right - windowRes.left;
                        ret.height = windowRes.bottom - windowRes.top;
                        ret.init = true;
                        ret.isWindowed = !shared::system::win32::isFullscreen(wnd, refMonitor, 4);
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

    _currentContainerFormat = containerFormat();
    LOG_INFO("Finish loading cache info." << std::endl);

    if (!_cachedRecordingSettings || (!_process.empty() && (!_cachedWindowInfo || !_cachedWindowInfo->init))) {
        THROW_ERROR("Failed to create cached info objects.");
    }
}

void GameRecorder::clearCachedInfo() {
    _cachedRecordingSettings.reset(nullptr);
    _cachedWindowInfo.reset(nullptr);
    _currentContainerFormat = "";
}

GameRecorder::EncoderDatum GameRecorder::createEncoder(size_t desiredWidth, size_t desiredHeight) {
    EncoderDatum data;

    LOG_INFO("Create FFmpeg Encoder" << std::endl);

    // We don't really need to pass in an stream URL (for now) this is primarily for determining what format the file output should be in.
    // By default, that should be MPEG-TS so no need to do anything fancy here unless some override has been given.
    data.encoder = std::make_unique<encoder::FfmpegAvEncoder>(_forcedOutputUrl ? _forcedOutputUrl.value() : "", _currentContainerFormat);

    // Assume that the input recorders have already been created before this point.
    // This is primarily for the audio inputs so we know how many inputs to expect.
    LOG_INFO("Initialize video stream..." << std::endl);
    data.encoder->initializeVideoStream(
        _compositor->context(),
        *_cachedRecordingSettings,
        desiredWidth,
        desiredHeight
    );

    LOG_INFO("Initialize audio stream..." << std::endl);
    const bool hasAudio = std::any_of(_aoutRecorder.begin(), _aoutRecorder.end(), [](const auto& r){ return r->exists(); })
        || std::any_of(_ainRecorder.begin(), _ainRecorder.end(), [](const auto& r){ return r->exists(); });

    if (hasAudio) {
        data.encoder->initializeAudioCodec();
        for (size_t i = 0; i < _aoutRecorder.size(); ++i) {
            if (_aoutRecorder[i]->exists()) {
                LOG_INFO("Adding audio output..." << std::endl);

                service::recorder::encoder::AudioInputSettings settings;
                settings.useSilenceCompensation = _cachedRecordingSettings->useAudioDriftCompensation;
                settings.useNoiseThreshold = false;
                settings.useSpeechNoiseReduction = false;

                const auto encoderIndex = data.encoder->addAudioInput(_aoutRecorder[i]->deviceName(), _aoutRecorder[i]->props(), settings);
                data.audioEncoderIndex[audio::EAudioDeviceDirection::Output][i] = encoderIndex;
            }
        }

        for (size_t i = 0; i < _ainRecorder.size(); ++i) {
            if (_ainRecorder[i]->exists()) {
                LOG_INFO("Adding audio input..." << std::endl);

                service::recorder::encoder::AudioInputSettings settings;
                settings.useSilenceCompensation = _cachedRecordingSettings->useAudioDriftCompensation;
                settings.useNoiseThreshold =  _ainRecorder[i]->isVoice() ? _cachedRecordingSettings->useVoiceBasicNoiseFilter : false;
                settings.noiseThresholDb = _cachedRecordingSettings->voiceFilterThresholdDb;
                settings.useSpeechNoiseReduction = _ainRecorder[i]->isVoice() ? _cachedRecordingSettings->useVoiceSpeechNoiseReduction : false;

                const auto encoderIndex = data.encoder->addAudioInput(_ainRecorder[i]->deviceName(), _ainRecorder[i]->props(), settings);
                data.audioEncoderIndex[audio::EAudioDeviceDirection::Input][i] = encoderIndex;
            }
        }
        data.encoder->initializeAudioStreams(_cachedRecordingSettings->useSeparateAudioChannels);
    }

    data.encoder->finalizeStreams();
    return data;
}

bool GameRecorder::areInputStreamsInitialized() const {
    return _streamsInit;
}

bool GameRecorder::initializeCompositor(const video::VideoWindowInfo& info, int flags, size_t desiredWidth, size_t desiredHeight) {
    LOG_INFO("Initialize compositor [Use GPU: " << _cachedRecordingSettings->useVideoHw2 << "]" << std::endl);
    _compositor = std::make_unique<service::recorder::compositor::Compositor>(
        _cachedRecordingSettings->useVideoHw2 ? service::renderer::D3d11Device::GPU : service::renderer::D3d11Device::CPU,
        desiredWidth,
        desiredHeight
    );

    bool allowMouse = true;
    if (_compositor) {
        LOG_INFO("...Creating clock layer." << std::endl);
        // First create the clock layer which will be driven by the video recorder (desktop duplication, WGC, GDI, etc.).
        // Note that we will need to build the graph that flows the video recorder into the the clock layer.
        auto clockLayer = _compositor->createClockLayer();

        _streamsInit = true;

        try {
            allowMouse = createVideoRecorder(info, flags);
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to create video recorder: " << ex.what() << std::endl);
            _streamsInit = false;
            return false;
        }
        _vrecorder->startRecording();

        // Should we be able to customize this pipeline?
        LOG_INFO("...Creating FPS limiter." << std::endl);
        auto fpsLimiter = std::make_shared<service::recorder::compositor::graph::FpsLimiterNode>(_cachedRecordingSettings->fps);
        _vrecorder->setNext(fpsLimiter);
        _fpsLimiter = fpsLimiter;

        LOG_INFO("...Creating texture normalizer." << std::endl);
        auto textureNormalizer = std::make_shared<service::recorder::compositor::graph::TextureContextNormalizerNode>(_compositor->context());
        fpsLimiter->setNext(textureNormalizer);

        LOG_INFO("...Creating sink node." << std::endl);
        auto sinkNode = std::make_shared<service::recorder::compositor::graph::SinkNode>();
        textureNormalizer->setNext(sinkNode);

        clockLayer->setSinkNode(sinkNode);
    } else {
        THROW_ERROR("Failed to allocate memory for compositor.");
        return false;
    }

    // Add in the different layers for the overlay if it exists and is enabled.
    const auto& overlaySettings = _cachedRecordingSettings->overlays;
    LOG_INFO("Checking if overlay enabled: " << overlaySettings.enabled);
    if (overlaySettings.enabled) {
        LOG_INFO("...Num overlay layers: " << overlaySettings.layers.size() << std::endl);
        // Parse through the overlay settings and for each layer create a new layer object.
        // Note that a 'layer' may actually be composed of multiple things we want to render.
        // I don't remember why I did it this way, bite me.
        for (size_t i = 0; i < overlaySettings.layers.size(); ++i) {
            const auto realIdx = overlaySettings.layers.size() - 1 - i;
            const auto& layer = overlaySettings.layers[i];

            if (!layer.enabled) {
                continue;
            }

            if (layer.games.find(_game) == layer.games.end()) {
                continue;
            }

            const auto newLayers = service::recorder::compositor::layers::createCompositorLayersForOverlayLayer(layer);
            for (const auto& nl: newLayers) {
                _compositor->addLayer(nl);
            }
        }
    }

    if (_cachedRecordingSettings->watermark.enabled) {
        LOG_INFO("...Adding watermark layer." << std::endl);
        _compositor->addLayer(
            std::make_shared<service::recorder::compositor::layers::WatermarkLayer>(_cachedRecordingSettings->watermark)
        );
    }

    // Mouse cursor layer should be last since it makes more sense that it should be rendered
    // on top of everything (game, overlay, etc).
    LOG_INFO("Check Record Mouse: " << (_cachedRecordingSettings->recordMouse2 && allowMouse) << std::endl);
    if (_cachedRecordingSettings->recordMouse2 && allowMouse) {
        // The mouse layer needs to know which window we're currently trying to record (the image that's being
        // sent to the clock layer) so that it knows how to properly position the mouse with respect to the
        // game image.
        auto mouseLayer = std::make_shared<service::recorder::compositor::layers::MouseCursorLayer>(_cachedWindowInfo->window);
        _compositor->addLayer(mouseLayer);
    }

    _compositor->finalizeLayers();
    return true;
}

bool GameRecorder::initializeInputStreams(int flags, size_t desiredWidth, size_t desiredHeight) {
#ifdef _WIN32
    // I think this is needed because we aren't generally calling startRecording on the same thread as Pa_Initialize?
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif

    _syncClockTime = service::recorder::encoder::AVSyncClock::now();
    initializeCompositor(*_cachedWindowInfo, flags, desiredWidth, desiredHeight);
    
    if (!_cachedRecordingSettings->useWASAPIRecording3) {
        std::lock_guard guard(_paInitMutex);
        _paInit.reset(new audio::PortaudioInitRAII);
    }

    service::recorder::audio::AudioDeviceSet deviceSet;
    
    if (_cachedRecordingSettings->useWASAPIRecording3 && _cachedRecordingSettings->usePerProcessRecording) {
        using RecordPair = std::pair<OSPID, double>;
        std::unordered_set<RecordPair, boost::hash<RecordPair>> finalExesToRecord;

        if (_cachedRecordingSettings->recordGameAudio) {
            finalExesToRecord.insert(std::make_pair(_process.pid(), _cachedRecordingSettings->gameAudioVolume));
        }

        auto itf = std::make_shared<shared::system::win32::interfaces::Win32SystemProcessInterface>();
        process_watcher::process::ProcessRunningState processState(itf);
        processState.update();

        for (const auto& pName: _auxGameProcessesForAudio) {
            const auto auxProcesses = processState.getProcesssRunningByName(shared::strings::utf8ToWcs(pName), false);
            for (const auto& pp: auxProcesses) {
                finalExesToRecord.insert(std::make_pair(pp.pid(), _cachedRecordingSettings->gameAudioVolume));
            }
        }

        for (const auto& p: _cachedRecordingSettings->processesToRecord) {
            // Need to find the EXE with a valid window - there could be multiple processes that satisfy this condition.
            // That's fine! We'll just record all of them to be safe. Yolo.
            const auto processes = processState.getProcesssRunningByName(shared::strings::utf8ToWcs(p.process.exe), true);
            for (const auto& pp: processes) {
                finalExesToRecord.insert(std::make_pair(pp.pid(), p.volume));
            }
        }

        for (const auto& exe: finalExesToRecord) {
            auto recorder = std::make_unique<audio::WasapiProgramRecorder>(exe.first, exe.second);

            try {
                if (recorder->exists()) {
                    recorder->startRecording();
                    _aoutRecorder.emplace_back(std::move(recorder));
                }
            } catch (std::exception& ex) {
                LOG_ERROR("Failed to record specified program: " << ex.what() << std::endl);
            }
        }
    } else {
        for (const auto& output : _cachedRecordingSettings->outputDevices) {
            std::unique_ptr<audio::AudioRecorder> recorder;
            if (_cachedRecordingSettings->useWASAPIRecording3) {
                recorder = std::make_unique<audio::WasapiAudioRecorder>();
            } else {
                recorder = std::make_unique<audio::PortaudioAudioRecorder>();
            }

            try {
                recorder->loadDevice(audio::EAudioDeviceDirection::Output, output, deviceSet);
                if (recorder->exists()) {
                    recorder->startRecording();
                    _aoutRecorder.emplace_back(std::move(recorder));
                }
            } catch (std::exception& ex) {
                LOG_ERROR("Failed to record output audio device: " << ex.what() << std::endl);
            }
        }
    }

    std::lock_guard guard(_ainMutex);
    for (const auto& input : _cachedRecordingSettings->inputDevices) {
        std::unique_ptr<audio::AudioRecorder> recorder;
        if (_cachedRecordingSettings->useWASAPIRecording3) {
            recorder = std::make_unique<audio::WasapiAudioRecorder>();
        } else {
            recorder = std::make_unique<audio::PortaudioAudioRecorder>();
        }

        try {
            recorder->loadDevice(audio::EAudioDeviceDirection::Input, input, deviceSet);
            if (recorder->exists()) {
                if (input.voice) {
                    recorder->markVoice();
                }

                recorder->startRecording();
                _ainRecorder.emplace_back(std::move(recorder));
            }
        } catch (std::exception& ex) {
            LOG_ERROR("Failed to record input audio device: " << ex.what() << std::endl);
        }
    }

    if (_cachedRecordingSettings->usePushToTalk) {
        {
            // If we're using PTT then we have to disable the voices by default.
            for (auto& r : _ainRecorder) {
                r->setVolume(0.0);
            }
        }

        _pttEnableCb = shared::system::win32::Win32MessageLoop::singleton()->addActionCallback(service::system::EAction::PushToTalkEnable, [this](){
            std::lock_guard guard(_ainMutex);
            for (auto& r : _ainRecorder) {
                r->setVolume(r->initialVolume());
            }
        });

        _pttDisableCb = shared::system::win32::Win32MessageLoop::singleton()->addActionCallback(service::system::EAction::PushToTalkDisable, [this](){
            std::lock_guard guard(_ainMutex);
            for (auto& r : _ainRecorder) {
                r->setVolume(0.0);
            }
        });
    }

    return true;
}

bool GameRecorder::isGameEnabled() const {
    return service::system::getCurrentSettings()->isGameEnabled(_game);
}

void GameRecorder::start(int flags) {
    if (_encoder.hasEncoder()) {
        return;
    }

    if (flags == FLAG_UNKNOWN) {
        flags = service::recorder::getDefaultRecordingFlagsForGame(_game);
    }

    loadCachedInfo();

    if (!isGameEnabled()) {
        LOG_WARNING("Ignoring VOD start since game is disabled: " << shared::gameToString(_game) << std::endl);
        return;
    }

    // Pulled the computation of desiredWidth and Height out to start since this info is needed by the compositor as well.
    // Use native monitor resolution if the user desires it instead of the game aspect ratio (useful for users who do stretched res).
    const auto nativeAspectRatio = shared::system::win32::getNativeMonitorAspectRatio(_cachedWindowInfo->monitor);
    
    // We only want to use the native aspect ratio if 1) the user wants it, 2) we computed it properly, and 3) the user is playing in full screen.
    // In the case of windowed mode gaming, we should keep the window size since this is mainly for when the user is playing in a different aspect ratio
    // than their monitor (e.g. stretched resolution for CSGO/Valorant).
    const auto aspectRatio = (_cachedRecordingSettings->useNativeAspectRatio && nativeAspectRatio && !_cachedWindowInfo->isWindowed) ?
        nativeAspectRatio.value():
        static_cast<double>(_cachedWindowInfo->width) / _cachedWindowInfo->height;
    const auto desiredHeight = shared::math::forceEven(_overrideHeight.value_or(std::min(_cachedWindowInfo->height, static_cast<size_t>(_cachedRecordingSettings->resY))));
    const auto desiredWidth = shared::math::forceEven(_overrideWidth.value_or(static_cast<size_t>(desiredHeight * aspectRatio)));
    LOG_INFO("Final Resolution: " << desiredWidth << "x" << desiredHeight << std::endl);

    if (!areInputStreamsInitialized()) {
        LOG_INFO("Initialize input streams..." << std::endl);
        std::future<bool> successFut = std::async(std::launch::async, &GameRecorder::initializeInputStreams, this, flags, desiredWidth, desiredHeight);
        if (!successFut.get()) {
            LOG_WARNING("Failed to start game recording." << std::endl);
            _currentId = {};
            _outputPiper->stop();
            return;
        }
    }

    LOG_INFO("Creating encoder..." << std::endl);
    _encoder = createEncoder(desiredWidth, desiredHeight);

    if (!service::system::getCurrentSettings()->keybinds().clip.keys.empty() && _cachedInstantClipLengthSeconds > 0) {
        LOG_INFO("Initializing initial DVR storage: " << _cachedInstantClipLengthSeconds << std::endl);
        initializeDvrOutput(static_cast<double>(_cachedInstantClipLengthSeconds));
    }
}

void GameRecorder::connectInputs() {
    if (!isGameEnabled()) {
        return;
    }
    LOG_INFO("Switch to new active encoder..." << std::endl);
    switchToNewActiveEncoder(_encoder);
}

void GameRecorder::initializeDvrOutput(double sizeSeconds) {
    if (!isGameEnabled()) {
        return;
    }

    if (!_encoder.hasEncoder()) {
        LOG_WARNING("Trying to initialize DVR output when there's no encoder." << std::endl);
        return;
    }

    LOG_INFO("Initialize DVR Output: " << sizeSeconds << std::endl);
    _encoder.encoder->resizeDvrBuffer(sizeSeconds);
}
void GameRecorder::initializeVideoOutput(const shared::TimePoint& start, RecordingMode mode) {
    if (!isGameEnabled()) {
        return;
    }

    if (_currentId) {
        return;
    }

    LOG_INFO("Request VOD Record Start: " << shared::timeToStr(start) << " - Mode: " << static_cast<int>(mode) << std::endl);
    service::api::getGlobalApi()->markUserAnalyticsEvent("start_record");

    _currentId = createNewVodIdentifier();
    try {
        initializeFileOutputPiper();
    } catch (std::exception& ex) {
        _currentId.reset(nullptr);
        LOG_ERROR("Failed to initialize output piper...ignoring start recording command: " << ex.what() << std::endl);
        return;
    }

    LOG_INFO("Hooking bookmark keybind..." << std::endl);
    {
        const auto bookmarkId = _currentId->videoUuid;
        _bookmarkCb = shared::system::win32::Win32MessageLoop::singleton()->addActionCallback(service::system::EAction::Bookmark, [this, bookmarkId](){
            service::api::getGlobalApi()->createBookmark(bookmarkId, shared::nowUtc());
            DISPLAY_NOTIFICATION(
                service::system::NotificationSeverity::Warning,
                service::system::NotificationDisplayType::NativeNotification,
                "SquadOV :: Bookmark Created",
                "You'll be able to find this moment later."
            );
        });
    }

    LOG_INFO("Hooking clipping keybind..." << std::endl);
    {
        const auto clipId = _currentId->videoUuid;
        _clipCb = shared::system::win32::Win32MessageLoop::singleton()->addActionCallback(service::system::EAction::Clip, [this, clipId](){
            LOG_INFO("Creating clip of length " << _cachedInstantClipLengthSeconds << " seconds..." << std::endl);
            if (_cachedRecordingSettings->useLocalRecording) {
                // If this VOD is locally recorded, keep track of the clipping requests and handle them after the game is finished.
                LOG_WARNING("...Currently unsupported when using local recording." << std::endl);
            } else {
                // If this VOD is automatically uploaded, create a staged clip.
                const auto end = shared::timeToUnixMs(shared::nowUtc()) - shared::timeToUnixMs(_vodStartTime);
                const auto start = std::max(end - _cachedInstantClipLengthSeconds * 1000, static_cast<int64_t>(0));
                service::api::getGlobalApi()->createStagedClip(clipId, start, end);
                DISPLAY_NOTIFICATION(
                    service::system::NotificationSeverity::Warning,
                    service::system::NotificationDisplayType::NativeNotification,
                    "SquadOV :: Clip Created",
                    "You'll be able to find this clip in your Clip Library after the game!"
                );
            }
        });
    }

    if (_outputPiper) {
        LOG_INFO("Start output piper..." << std::endl);
        _outputPiper->start();
    }

    LOG_INFO("Determining DVR session start..." << std::endl);
    std::optional<service::recorder::encoder::AVSyncClock::time_point> dvrStart;
    if (mode == RecordingMode::DVR) {
        dvrStart = shared::convertClockTime<shared::TimePoint, service::recorder::encoder::AVSyncClock::time_point>(start);
    }

    LOG_INFO("Open file for encoder output..." << std::endl);
    if (_forcedOutputUrl) {
        _vodStartTime = _encoder.encoder->open(_forcedOutputUrl.value(), dvrStart);
    } else if (_outputPiper) {
        _vodStartTime =_encoder.encoder->open(_outputPiper->filePath(), dvrStart);
    } else {
        THROW_ERROR("No valid URL to output video to.");
    }

    LOG_INFO("Final VOD Start Time: " << shared::timeToStr(_vodStartTime) << std::endl);
    system::getGlobalState()->markGameRecording(_game, true);
}

void GameRecorder::stopInputs() {
    _streamsInit = false;

    if (_vrecorder) {
        LOG_INFO("...Stop video recorder." << std::endl);
        _vrecorder->stopRecording();
        LOG_INFO("\tOK" << std::endl);
        _vrecorder.reset(nullptr);
    }

    for (auto& recorder : _aoutRecorder) {
        LOG_INFO("...Stop audio output recorder." << std::endl);
        recorder->stop();
    }

    LOG_INFO("\tOK" << std::endl);
    _aoutRecorder.clear();

    {
        // Remove PTT callbacks first as the callbacks will want a lock on audio input so we want to make sure we don't
        // let that get in the way of stopping the audio input recorder.
        if (_pttEnableCb.has_value()) {
            LOG_INFO("...Remove PTT enable callback." << std::endl);
            shared::system::win32::Win32MessageLoop::singleton()->removeActionCallback(service::system::EAction::PushToTalkEnable, _pttEnableCb.value());
            LOG_INFO("\tOK" << std::endl);
            _pttEnableCb.reset();   
        }

        if (_pttDisableCb.has_value()) {
            LOG_INFO("...Remove PTT disable callback." << std::endl);
            shared::system::win32::Win32MessageLoop::singleton()->removeActionCallback(service::system::EAction::PushToTalkDisable, _pttDisableCb.value());
            LOG_INFO("\tOK" << std::endl);
            _pttDisableCb.reset();
        }

        std::lock_guard guard(_ainMutex);
        for (auto& recorder : _ainRecorder) {
            LOG_INFO("...Stop audio input recorder." << std::endl);
            recorder->stop();
        }

        LOG_INFO("\tOK" << std::endl);
        _ainRecorder.clear();
    }

    {
        std::lock_guard guard(_paInitMutex);
        _paInit.reset(nullptr);
    }

#ifdef _WIN32
    CoUninitialize();
#endif
}

void GameRecorder::stop(std::optional<GameRecordEnd> end, bool keepLocal) {
    std::lock_guard guard(_stopMutex);
    LOG_INFO("Stop Game Recording...Clearing VOD ID" << std::endl);
    const auto vodId = _currentId ? currentId() : VodIdentifier{};
    const auto metadata = getMetadata();
    _currentId.reset(nullptr);

    // Only do the delay if we're going to be uploading the VOD. If we're not going to be uploading
    // the VOD then it doesn't matter what delay there is.
    if (end && _cachedRecordingSettings && _cachedRecordingSettings->vodEndDelaySeconds > 0) {
        LOG_INFO("VOD Stop Delay: " << _cachedRecordingSettings->vodEndDelaySeconds << std::endl);
        const auto delay = std::chrono::seconds(_cachedRecordingSettings->vodEndDelaySeconds);
        auto modifiedEnd = end.value();
        modifiedEnd.endTime += delay;
        std::this_thread::sleep_for(delay);
        end = modifiedEnd;
    }

    if (_bookmarkCb.has_value()) {
        LOG_INFO("...Removing bookmark hook..." << std::endl);
        shared::system::win32::Win32MessageLoop::singleton()->removeActionCallback(service::system::EAction::Bookmark, _bookmarkCb.value());
        LOG_INFO("\tOK" << std::endl);
        _bookmarkCb.reset();
    }

    if (_clipCb.has_value()) {
        LOG_INFO("...Removing clip hook..." << std::endl);
        shared::system::win32::Win32MessageLoop::singleton()->removeActionCallback(service::system::EAction::Clip, _clipCb.value());
        LOG_INFO("\tOK" << std::endl);
        _clipCb.reset();
    }

    LOG_INFO("Stop Inputs..." << std::endl);
    stopInputs();

    if (!_cachedRecordingSettings) {
        LOG_INFO("...Cached recording settings already cleared - ignoring recording stop." << std::endl);
        return;
    }

    LOG_INFO("Retrieving local recording info..." << std::endl);
    const auto wasLocal = _cachedRecordingSettings->useLocalRecording;
    const auto maxLocalSize = _cachedRecordingSettings->maxLocalRecordingSizeGb;
    const auto singleton = shared::filesystem::LocalRecordingIndexDb::singleton();
    const auto wasFormat = _currentContainerFormat;
    singleton->initializeFromFolder(_cachedRecordingSettings->localRecordingLocation);

    LOG_INFO("Clear Cached Info..." << std::endl);
    clearCachedInfo();
    if (!isRecording()) {
        LOG_INFO("...Not recording - ignoring recording stop." << std::endl);
        return;
    }

    const auto vodStartTime = this->vodStartTime();
    
    if (_encoder.hasEncoder()) {
        LOG_INFO("Stop primary encoder..." << std::endl);
        _encoder.encoder->stop();
        _encoder = {};
    }

    // Need to clear the compositor after the encoder since the encoder will
    // be using the compositor's D3D context.
    if (_compositor) {
        LOG_INFO("...Clear compositor." << std::endl);
        _compositor.reset(nullptr);
    }

    if (_outputPiper) {
        LOG_INFO("Stop output piper..." << std::endl);
        const auto sessionId = this->sessionId();

        // Move the output piper to a new thread to wait for it to finish
        // so we don't get bottlenecked by any user's poor internet speeds.
        // We only do VOD association when the upload ends so we don't tell the
        // server to associate a VOD that doesn't actually exist.
        std::thread uploadThread([this, vodId, metadata, sessionId, vodStartTime, end, wasLocal, wasFormat, singleton, keepLocal, maxLocalSize](){
            pipe::FileOutputPiperPtr outputPiper = std::move(_outputPiper);
            _outputPiper.reset(nullptr);
            outputPiper->wait();

            if (end.has_value()) {
                shared::squadov::VodAssociation association;
                association.matchUuid = end.value().matchUuid;
                association.userUuid = service::api::getGlobalApi()->getCurrentUserCached().uuid;
                association.videoUuid = vodId.videoUuid;
                association.startTime = vodStartTime;
                association.endTime = end.value().endTime;
                association.rawContainerFormat = (wasFormat == "webm") ? "webm" : wasLocal ? "mp4" : "mpegts";
                association.isLocal = wasLocal;

                try {
                    service::api::getGlobalApi()->associateVod(association, metadata, sessionId, outputPiper->segmentIds());
                    if (wasLocal && outputPiper->localFile().has_value()) {
                        const auto rawPath = outputPiper->localFile().value();
                        const auto processedPath = shared::filesystem::getSquadOvTempFolder() / fs::path(association.videoUuid) / ((wasFormat == "webm") ? fs::path("video.webm") : fs::path("video.mp4"));
                        service::vod::processRawLocalVod(rawPath, processedPath, (wasFormat == "webm") ? "webm" : "mpegts");

                        if (!fs::exists(processedPath)) {
                            THROW_ERROR("Failed find locally processed video at: " << processedPath << std::endl);
                        }

                        // We need to process this VOD locally (aka do the equivalent of the server side fastify).
                        shared::filesystem::LocalRecordingIndexEntry entry;
                        entry.uuid = association.videoUuid;
                        entry.relative = entry.uuid + ((wasFormat == "webm") ? ".webm" : ".mp4");

                        // After that's complete, we can finally add this file to the local index.
                        singleton->addLocalEntryFromFilesystem(processedPath, entry);

                        // Need to cleanup the local recording folder after adding a new entry.
                        singleton->cleanupLocalFolder(maxLocalSize);
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

    if (_outputPiper) {
        _outputPiper->start();
    }

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

        // This is kind of hacky to handle instances where we timeout and we might not get a proper timeout time. Yolo.
        const auto toMs = startMs + std::min(std::chrono::duration_cast<std::chrono::milliseconds>(endTm - _manualVodTimeStart).count() - startMs, static_cast<int64_t>(30 * 60 * 1000));

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


    if (!end.matchUuid.empty())  {
        shared::squadov::VodAssociation association;
        association.matchUuid = end.matchUuid;
        association.userUuid = service::api::getGlobalApi()->getCurrentUserCached().uuid;
        association.videoUuid = vodId.videoUuid;
        association.startTime = vodStartTime;
        association.endTime = end.endTime;
        association.rawContainerFormat = "mpegts";

        try {
            service::api::getGlobalApi()->associateVod(association, metadata, sessionId, _outputPiper->segmentIds());
        } catch (std::exception& ex) {
            LOG_WARNING("Failed to associate VOD: " << ex.what() << std::endl);
            service::api::getGlobalApi()->deleteVod(vodId.videoUuid);
        }
    }

    _outputPiper.reset(nullptr);
}

std::unique_ptr<VodIdentifier> GameRecorder::createNewVodIdentifier() const {
    auto id = std::make_unique<VodIdentifier>();
    id->videoUuid = shared::generateUuidv4();
    return id;
}

std::string GameRecorder::containerFormat() const {
    std::string formatHint = "mpegts";

    if (service::api::getGlobalApi()->getSessionFeatures().allowVp9) {
        switch (_cachedRecordingSettings->videoCodec) {
            case service::recorder::video::VideoCodec::VP9:
                formatHint = "webm";
                break;
        }
    }
    return formatHint;
}

void GameRecorder::initializeFileOutputPiper() {
    if (_outputPiper) {
        return;
    }

    if (_forcedOutputUrl) {
        return;
    }

    // Create a pipe to the destination file. Could be a Google Cloud Storage signed URL
    // or even a filesystem. The API will tell us the right place to pipe to - we'll need to
    // create an output piper of the appropriate type based on the given URI.
    const auto cloudDestination = service::api::getGlobalApi()->createVodDestinationUri(_currentId->videoUuid, _currentContainerFormat);
    const auto localTmpRecord = shared::filesystem::getSquadOvTempFolder()  / fs::path(_currentId->videoUuid) / ((_currentContainerFormat == "webm") ? fs::path("temp.webm") : fs::path("temp.ts"));
    const auto finalDestination = _cachedRecordingSettings->useLocalRecording ? service::uploader::UploadDestination::local(localTmpRecord) : cloudDestination;
    setFileOutputFromDestination(_currentId->videoUuid, finalDestination);
}

void GameRecorder::setFileOutputFromDestination(const std::string& videoUuid, const service::uploader::UploadDestination& destination) {
    LOG_INFO("Output Destination: " << destination << std::endl);
    _outputDestination = destination;
    _outputPiper = pipe::createFileOutputPiper(videoUuid, destination);

    if (_cachedRecordingSettings && _cachedRecordingSettings->bandwidthLimiterMultiple) {
        const auto bytesPerSec = static_cast<size_t>(_cachedRecordingSettings->bitrateKbps * (_cachedRecordingSettings->bandwidthLimiterMultiple.value() / 100.0) * 1000 / 8.0);
        LOG_INFO("...Using Bandwidth Limiter: " << bytesPerSec << " bytes per second" << std::endl);
        _outputPiper->setMaxUploadSpeed(bytesPerSec);
    }
}

shared::squadov::VodMetadata GameRecorder::getMetadata() const {
    std::string bucket = "";
    std::optional<std::string> session = std::nullopt;

    if (_outputDestination) {
        const auto dest = _outputDestination.value();
        bucket = _outputDestination->bucket;
        session = _outputDestination->session;
    }

    if (_encoder.encoder) {
        auto metadata = _encoder.encoder->getMetadata();
        metadata.videoUuid = _currentId ? _currentId->videoUuid : "";
        metadata.bucket = bucket;
        metadata.sessionId = session;
        return metadata;
    } else {
        shared::squadov::VodMetadata metadata;
        metadata.id = "source";
        metadata.videoUuid = _currentId ? _currentId->videoUuid : "";
        metadata.bucket = bucket;
        metadata.sessionId = session;
        
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

void GameRecorder::enableOverlay(bool enable) {
    for (const auto& layer: _overlayLayers) {
        layer->enable(enable);
    }
}

}