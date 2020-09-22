#include "recorder/audio/portaudio_audio_recorder.h"

#include "shared/errors/error.h"
#include "recorder/encoder/av_encoder.h"
#include "recorder/audio/audio_packet_view.h"

#include <iostream>

#ifdef _WIN32
#include <objbase.h>
#endif

extern "C" {
#include <portaudio.h>
}

namespace service::recorder::audio {

constexpr int numChannels = 2;

class PortaudioAudioRecorderImpl {
public:
    explicit PortaudioAudioRecorderImpl(EAudioDeviceDirection dir);
    ~PortaudioAudioRecorderImpl();

    void startRecording(service::recorder::encoder::AvEncoder* encoder, size_t encoderIdx);
    void stop();

    bool exists() const { return _exists; }
    const AudioPacketProperties& props() { return _props; }

    int portaudioCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);
private:

    EAudioDeviceDirection _dir;
    bool _exists = false;    

    size_t _sampleRate = 0;
    PaStreamParameters _streamParams;
    PaStream* _stream = nullptr;
    AudioPacketProperties _props;

    service::recorder::encoder::AvEncoder* _encoder = nullptr;
    size_t _encoderIdx = 0;

    PaTime _startTime;
    service::recorder::encoder::AVSyncClock::time_point _syncStartTime;
};

int gPortaudioCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
    auto* impl = reinterpret_cast<PortaudioAudioRecorderImpl*>(userData);
    return impl->portaudioCallback(input, output, frameCount, timeInfo, statusFlags);
}

PortaudioAudioRecorderImpl::PortaudioAudioRecorderImpl(EAudioDeviceDirection dir):
    _dir(dir) {

    // Use the default input/output devices for now.
    PaDeviceIndex defaultDevice = 
        (dir == EAudioDeviceDirection::Input) ?
            Pa_GetDefaultInputDevice() :
            Pa_GetDefaultOutputDevice();

    if (defaultDevice == paNoDevice) {
        return;
    }

    const PaDeviceInfo* di = Pa_GetDeviceInfo(defaultDevice);
    const std::string diName(di->name);

    // In the case we want to capture an output device we need to find the equivalent loopback device.
    if (dir == EAudioDeviceDirection::Output) {
        const std::string loopbackSuffix = "(loopback)";
        for (auto i = 0; i < Pa_GetDeviceCount(); ++i) {
            const PaDeviceInfo* ldi = Pa_GetDeviceInfo(i);
            const std::string ldiName(ldi->name);
            if (ldiName.substr(0, diName.size()) == diName &&
                ldiName.substr(ldiName.size() - loopbackSuffix.size()) == loopbackSuffix) {

                defaultDevice = i;
                di = ldi;
                break;
            }
        }
    }

    // At this point the input device info MUST have input channels or else we wouldn't be able
    // to record from it.
    if (!di->maxInputChannels) {
        return;
    }

    _exists = true;
    std::cout << "Found Audio Device [" << audioDeviceDirectionToStr(_dir) << "]: " << di->name << " CHANNELS: " << di->maxInputChannels << std::endl;
    _streamParams.device = defaultDevice;
    _streamParams.suggestedLatency = di->defaultLowInputLatency;
    _streamParams.hostApiSpecificStreamInfo = nullptr;
    _streamParams.channelCount = numChannels;
    _streamParams.sampleFormat = paFloat32;
    _sampleRate = static_cast<size_t>(di->defaultSampleRate);

    // Setup packet properties so that we can communicate with the encoder as to what format the packets will be coming in.
    _props.isPlanar = false;
    _props.numChannels = numChannels;
    _props.numSamples = 0;
    _props.samplingRate = _sampleRate;
}

PortaudioAudioRecorderImpl::~PortaudioAudioRecorderImpl() {

}

int PortaudioAudioRecorderImpl::portaudioCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags) {
    if (!_encoder) {
        return paContinue;
    }

    // The total number of frames we're being sent here is frameCount * numChannels.
    // Create audio packet and send it to the encoder.
    const float* buffer = reinterpret_cast<const float*>(input);

    // Compute the input time using the AVSyncClock.
    const auto elapsedMs = std::chrono::milliseconds(static_cast<int64_t>((timeInfo->currentTime - _startTime) * 1000.0)); 
    const auto inputTime = _syncStartTime + elapsedMs;

    // Need to change the number of packets since the number of frames isn't fixed.
    AudioPacketProperties packetProps = props();
    packetProps.numSamples = static_cast<size_t>(frameCount);

    FAudioPacketView view(buffer, packetProps);
    _encoder->addAudioFrame(view, _encoderIdx, inputTime);
    return paContinue;
}

void PortaudioAudioRecorderImpl::startRecording(service::recorder::encoder::AvEncoder* encoder, size_t encoderIdx) {  
#ifdef _WIN32
    // I think this is needed because we aren't generally calling startRecording on the same thread as Pa_Initialize?
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif

    _encoder = encoder;
    _encoderIdx = encoderIdx;

    const PaDeviceInfo* di = Pa_GetDeviceInfo(_streamParams.device);

    {
        const auto err = Pa_OpenStream(&_stream, &_streamParams, nullptr, static_cast<double>(_sampleRate), paFramesPerBufferUnspecified, paNoFlag , gPortaudioCallback, (void*)this);
        if (err != paNoError) {
            THROW_ERROR("Failed to open port audio stream: " << err);
        }
    }

    _startTime = Pa_GetStreamTime(_stream);
    _syncStartTime = service::recorder::encoder::AVSyncClock::now();

    {
        const auto err = Pa_StartStream(_stream);
        if (err != paNoError) {
            THROW_ERROR("Failed to start stream: " << err);
        }
    }

}

void PortaudioAudioRecorderImpl::stop() {
    Pa_AbortStream(_stream);
    Pa_CloseStream(_stream);
}

PortaudioAudioRecorder::PortaudioAudioRecorder(EAudioDeviceDirection dir):
    _impl(new PortaudioAudioRecorderImpl(dir)) {

}

PortaudioAudioRecorder::~PortaudioAudioRecorder() = default;

void PortaudioAudioRecorder::startRecording(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) {
    _impl->startRecording(encoder, encoderIndex);
}

void PortaudioAudioRecorder::stop() {
    _impl->stop();
}

bool PortaudioAudioRecorder::exists() const {
    return _impl->exists();
}

const AudioPacketProperties& PortaudioAudioRecorder::props() const {
    return _impl->props();
}

}