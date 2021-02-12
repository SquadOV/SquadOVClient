#include "recorder/audio/portaudio_audio_recorder.h"

#include "shared/errors/error.h"
#include "shared/log/log.h"
#include "recorder/encoder/av_encoder.h"
#include "recorder/audio/audio_packet_view.h"
#include "recorder/audio/fixed_size_audio_packet.h"

#include <boost/lockfree/spsc_queue.hpp>
#include <iostream>
#include <mutex>

#ifdef _WIN32
#include <objbase.h>
#endif

extern "C" {
#include <portaudio.h>
}

namespace service::recorder::audio {

constexpr size_t audioSamplesPerFrame = 1024;
using AudioPacket = FixedSizeAudioPacket<float, audioSamplesPerFrame>;
using AudioPacketQueue = boost::lockfree::spsc_queue<AudioPacket, boost::lockfree::capacity<4096>, boost::lockfree::fixed_sized<true>>;
using FAudioPacketView = AudioPacketView<float>;

class PortaudioAudioRecorderImpl {
public:
    explicit PortaudioAudioRecorderImpl(EAudioDeviceDirection dir);
    ~PortaudioAudioRecorderImpl();

    void startRecording();
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex);
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
    std::mutex _encoderMutex;

    PaTime _startTime;
    service::recorder::encoder::AVSyncClock::time_point _syncStartTime;

    bool _running = false;
    std::thread _packetThread;
    AudioPacketQueue _packetQueue;
    void addToPacketQueue(const FAudioPacketView& view, const service::recorder::encoder::AVSyncClock::time_point& tm);
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
    LOG_INFO("Found Audio Device [" << audioDeviceDirectionToStr(_dir) << "]: " << di->name << " CHANNELS: " << di->maxInputChannels << std::endl);
    _streamParams.device = defaultDevice;
    _streamParams.suggestedLatency = di->defaultLowInputLatency;
    _streamParams.hostApiSpecificStreamInfo = nullptr;
    _streamParams.channelCount = di->maxInputChannels;
    _streamParams.sampleFormat = paFloat32;
    _sampleRate = static_cast<size_t>(di->defaultSampleRate);

    // Setup packet properties so that we can communicate with the encoder as to what format the packets will be coming in.
    _props.isPlanar = false;
    _props.numChannels = _streamParams.channelCount;
    _props.numSamples = 0;
    _props.samplingRate = _sampleRate;
}

PortaudioAudioRecorderImpl::~PortaudioAudioRecorderImpl() {

}

int PortaudioAudioRecorderImpl::portaudioCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags) {
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
    // It's not safe to do mutex operations here so we need to copy all this data into a queue that another thread
    // reads from to feed to the encoder.
    addToPacketQueue(view, inputTime);

    return paContinue;
}

void PortaudioAudioRecorderImpl::addToPacketQueue(const FAudioPacketView& view, const service::recorder::encoder::AVSyncClock::time_point& tm) {
    const auto samplesPerPacket = AudioPacket::N / view.props().numChannels;
    const auto numQueuePackets = (view.props().numSamples + samplesPerPacket - 1) / samplesPerPacket;
    for (std::remove_const_t<decltype(numQueuePackets)> i = 0; i < numQueuePackets; ++i) {
        // Start and end indices of the samples in the input packet view to store in the new fixed size packet. End index is non-inculsive.
        const auto start = i * samplesPerPacket;
        const auto end = (i == numQueuePackets - 1) ?
            view.props().numSamples :
            (i + 1) * samplesPerPacket;

        // We need to change numSamples to be how many samples we actually store in this packet.
        service::recorder::audio::AudioPacketProperties props = view.props();
        props.numSamples = end - start; 

        AudioPacket packet(props, tm);
        packet.copyFrom(view, start * view.props().numChannels, end * view.props().numChannels);
        _packetQueue.push(packet);
    }
}

void PortaudioAudioRecorderImpl::startRecording() {  
#ifdef _WIN32
    // I think this is needed because we aren't generally calling startRecording on the same thread as Pa_Initialize?
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif

    const PaDeviceInfo* di = Pa_GetDeviceInfo(_streamParams.device);
    if (!di) {
        THROW_ERROR("Failed to get device info: " << _streamParams.device << "/" << Pa_GetDeviceCount() << std::endl);
    }
    LOG_INFO("Start recording device: " << di->name << " (" << _streamParams.device << "/" << Pa_GetDeviceCount() << ")" << std::endl);

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

    _running = true;
    _packetThread = std::thread([this](){
        while (_running) {
            _packetQueue.consume_all([this](const AudioPacket& packet){
                std::lock_guard<std::mutex> guard(_encoderMutex);
                FAudioPacketView view(packet.buffer(), packet.props());
                if (_encoder) {
                    _encoder->addAudioFrame(view, _encoderIdx, packet.syncTime());
                }
            });
            // Need this sleep in here to reduce contetion on the lockfree queues.
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }
    });
}

void PortaudioAudioRecorderImpl::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) {
    std::lock_guard<std::mutex> guard(_encoderMutex);
    _encoder = encoder;
    _encoderIdx = encoderIndex;
}

void PortaudioAudioRecorderImpl::stop() {
    Pa_AbortStream(_stream);
    Pa_CloseStream(_stream);

    _running = false;
    if (_packetThread.joinable()) {
        _packetThread.join();
    }
}

PortaudioAudioRecorder::PortaudioAudioRecorder(EAudioDeviceDirection dir):
    _impl(new PortaudioAudioRecorderImpl(dir)) {

}

PortaudioAudioRecorder::~PortaudioAudioRecorder() = default;

void PortaudioAudioRecorder::startRecording() {
    _impl->startRecording();
}

void PortaudioAudioRecorder::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) {
    _impl->setActiveEncoder(encoder, encoderIndex);
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