#include "recorder/audio/portaudio_audio_recorder.h"

#include "shared/errors/error.h"
#include "shared/log/log.h"
#include "shared/audio/timing.h"
#include "recorder/encoder/av_encoder.h"
#include "recorder/audio/audio_packet_view.h"
#include "recorder/audio/fixed_size_audio_packet.h"

#include <atomic>
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

PortaudioInitRAII::PortaudioInitRAII() {
    LOG_INFO("Initialize PortAudio..." << std::endl);
    const auto paErr =  Pa_Initialize();
    if (paErr != paNoError) {
        if (paErr == paUnanticipatedHostError) {
            const auto* hostErr = Pa_GetLastHostErrorInfo();
            LOG_ERROR("PortAudio Host Error: " << hostErr->errorCode << "\t" << hostErr->errorText << std::endl);
            LOG_ERROR("\tPort Audio Host Api Type: " << hostErr->hostApiType << std::endl);
        }
        THROW_ERROR("Failed to initialize PortAudio: " << paErr << "\t" << Pa_GetErrorText(paErr));
    }
}

PortaudioInitRAII::~PortaudioInitRAII() {
    LOG_INFO("Terminate PortAudio..." << std::endl);
    const auto paErr = Pa_Terminate();
    if (paErr != paNoError) {
        LOG_ERROR("Failed to terminate PortAudio: " << paErr << "\t" << Pa_GetErrorText(paErr));
    }
}

constexpr size_t audioSamplesPerFrame = 1024;
using AudioPacket = FixedSizeAudioPacket<float, audioSamplesPerFrame>;
using AudioPacketQueue = boost::lockfree::spsc_queue<AudioPacket, boost::lockfree::capacity<4096>, boost::lockfree::fixed_sized<true>>;
using FAudioPacketView = AudioPacketView<float>;

class PortaudioAudioRecorderImpl {
public:
    PortaudioAudioRecorderImpl();
    ~PortaudioAudioRecorderImpl();

    void startRecording();
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex);
    void stop();
    void loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet);
    double initialVolume() const;
    void setVolume(double volume);

    bool exists() const { return _exists; }
    const AudioPacketProperties& props() { return _props; }

    int portaudioCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);
private:

    EAudioDeviceDirection _dir;
    bool _exists = false;
    std::atomic<double> _volume = 1.0;
    double _initialVolume = 1.0;
    bool _mono = false;

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

PortaudioAudioRecorderImpl::PortaudioAudioRecorderImpl() {
}

PortaudioAudioRecorderImpl::~PortaudioAudioRecorderImpl() {

}

void PortaudioAudioRecorderImpl::loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet) {
    LOG_INFO("Load Selected Audio Device: '" << device.device << "(" << device.id << ")" << "' @ " << device.volume << " [Mono: " << device.mono << "]" << std::endl);

    // Try to find the selected device. If we can't find it then use the default device.
    PaDeviceIndex selectedDevice = paNoDevice;
    for (PaDeviceIndex i = 0; i < Pa_GetDeviceCount() ; ++i) {
        const PaDeviceInfo* ldi = Pa_GetDeviceInfo(i);
        const std::string ldiName(ldi->name);

        const PaHostApiInfo* hostInfo = Pa_GetHostApiInfo(ldi->hostApi);
        if (hostInfo->type != paWASAPI) {
            continue;
        }

        if ((dir == EAudioDeviceDirection::Output && ldi->maxOutputChannels == 0) || (dir == EAudioDeviceDirection::Input && ldi->maxInputChannels == 0)) {
            continue;
        }

        if (ldiName.find("(loopback)") != std::string::npos) {
            continue;
        }

        if (ldiName == device.device) {
            LOG_INFO("...Found selected device!" << std::endl);
            selectedDevice = i;
            break;
        }
    }

    // Use the default device as fallback.
    bool usingDefault = false;
    if (selectedDevice == paNoDevice) {
        selectedDevice =
            (dir == EAudioDeviceDirection::Input) ?
                Pa_GetDefaultInputDevice() :
                Pa_GetDefaultOutputDevice();
        usingDefault = true;
        LOG_INFO("...Falling back to default device." << std::endl);
    }

    // If we still have no device then just don't record from this input.
    if (selectedDevice == paNoDevice) {
        return;
    }

    const PaDeviceInfo* di = Pa_GetDeviceInfo(selectedDevice);
    const std::string diName(di->name);

    // In the case we want to capture an output device we need to find the equivalent loopback device.
    std::string selectedName = diName;
    if (dir == EAudioDeviceDirection::Output) {
        const std::string loopbackSuffix = "(loopback)";
        for (auto i = 0; i < Pa_GetDeviceCount(); ++i) {
            const PaDeviceInfo* ldi = Pa_GetDeviceInfo(i);
            const std::string ldiName(ldi->name);
            if (ldiName.substr(0, diName.size()) == diName &&
                ldiName.substr(ldiName.size() - loopbackSuffix.size()) == loopbackSuffix) {

                selectedDevice = i;
                selectedName = ldiName;
                di = ldi;
                break;
            }
        }
    }

    // If we've already added this device then it'd be silly to try to record it again.
    if (deviceSet.find(selectedName) != deviceSet.end()) {
        LOG_WARNING("...Duplicate device detected. Ignoring." << std::endl);
        return;
    }

    // At this point the input device info MUST have input channels or else we wouldn't be able
    // to record from it.
    if (!di->maxInputChannels) {
        return;
    }

    LOG_INFO("Found Audio Device [" << audioDeviceDirectionToStr(dir) << "]: " << di->name << " CHANNELS: " << di->maxInputChannels << std::endl);
    _streamParams.device = selectedDevice;
    _streamParams.suggestedLatency = di->defaultLowInputLatency;
    _streamParams.hostApiSpecificStreamInfo = nullptr;
    _streamParams.channelCount = di->maxInputChannels;
    _streamParams.sampleFormat = paFloat32;
    _sampleRate = static_cast<size_t>(di->defaultSampleRate);

    if (Pa_IsFormatSupported(&_streamParams, nullptr, static_cast<double>(_sampleRate)) != paNoError) {
        LOG_WARNING("Selected format is unsupported for device: " << std::endl
            << "\tSample Rate: " << _sampleRate << std::endl);
        return;
    }

    // Setup packet properties so that we can communicate with the encoder as to what format the packets will be coming in.
    _props.isPlanar = false;
    _props.numChannels = device.mono ? 1 : _streamParams.channelCount;
    _props.numSamples = 0;
    _props.samplingRate = _sampleRate;
    _props.forceMono = device.mono;
    
    LOG_INFO("Open recording device: " << di->name << " (" << _streamParams.device << "/" << Pa_GetDeviceCount() << ")" << std::endl
        << "\tChannels: " << _streamParams.channelCount << std::endl
        << "\tSample Rate: " << _sampleRate << std::endl
        << "\tLatency: " << _streamParams.suggestedLatency << std::endl
    );

    for (int i = 0; !_exists && i < 2; ++i) {
        const auto err =  Pa_OpenStream(&_stream, &_streamParams, nullptr, static_cast<double>(_sampleRate), paFramesPerBufferUnspecified, paNoFlag , gPortaudioCallback, (void*)this);
        if (err != paNoError) {
            LOG_ERROR("Failed to open port audio stream: " << err << std::endl);

            if (_streamParams.channelCount > 2) {
                // Fallback number one is to use less channels if we have >2 channels.
                // Sometimes we're just given bogus information that's not true from PortAudio.
                LOG_WARNING("Fallback from " << _streamParams.channelCount << " to 2 channels." << std::endl);
                _streamParams.channelCount = 2;

                if (!device.mono) {
                    _props.numChannels = _streamParams.channelCount;
                }
                continue;
            } else if (!usingDefault) {
                LOG_WARNING("Falling back to default device." << std::endl);
                service::system::AudioDeviceSettings defaultDevice;
                defaultDevice.volume = device.volume;
                defaultDevice.mono = device.mono;
                loadDevice(dir, defaultDevice, deviceSet);
                break;
            } else {
                LOG_WARNING("No audio devices to fall back to...ignoring this audio stream." << std::endl);
                break;
            }
        } else {
            _exists = true;
            _volume = device.volume;
            _initialVolume = device.volume;
            _mono = device.mono;

            deviceSet.insert(selectedName);
        }
    }
}

int PortaudioAudioRecorderImpl::portaudioCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags) {
    // The total number of frames we're being sent here is frameCount * numChannels.
    // Create audio packet and send it to the encoder.
    const float* buffer = reinterpret_cast<const float*>(input);
    const auto inputTime = service::recorder::encoder::AVSyncClock::now();

    // Need to change the number of packets since the number of frames isn't fixed.
    AudioPacketProperties packetProps = props();
    packetProps.numSamples = static_cast<size_t>(frameCount);
    // This gets changed because the output packet might need to be mono while we aren't necessarily recording mono.
    packetProps.numChannels = _streamParams.channelCount;

    FAudioPacketView view(buffer, packetProps, _volume.load());
    // It's not safe to do mutex operations here so we need to copy all this data into a queue that another thread
    // reads from to feed to the encoder.
    addToPacketQueue(view, inputTime);

    return paContinue;
}

void PortaudioAudioRecorderImpl::addToPacketQueue(const FAudioPacketView& view, const service::recorder::encoder::AVSyncClock::time_point& tm) {
    const auto samplesPerPacket = AudioPacket::N / view.props().numChannels;
    const auto numQueuePackets = (view.props().numSamples + samplesPerPacket - 1) / samplesPerPacket;

    auto packetTm = tm;
    for (std::remove_const_t<decltype(numQueuePackets)> i = 0; i < numQueuePackets; ++i) {
        // Start and end indices of the samples in the input packet view to store in the new fixed size packet. End index is non-inculsive.
        const auto start = i * samplesPerPacket;
        const auto end = (i == numQueuePackets - 1) ?
            view.props().numSamples :
            (i + 1) * samplesPerPacket;

        // We need to change numSamples to be how many samples we actually store in this packet.
        service::recorder::audio::AudioPacketProperties props = view.props();
        props.numSamples = end - start;
        props.numChannels = _props.numChannels;

        AudioPacket packet(props, packetTm);
        packet.copyFrom(view, start * view.props().numChannels, end * view.props().numChannels);
        _packetQueue.push(packet);

        // Each packet's timestamp should be adjusted based off how long the previous packet was
        // so that we treat each packet separately. To do this adjustment, remember that
        // the sampling frequency is X samples / second. Thus to get the number of seconds all we
        // have to do is do N / X where N is the number of samples.
        packetTm += shared::audio::samplesToNsDiff(view.props().samplingRate, props.numSamples);
    }
}

void PortaudioAudioRecorderImpl::startRecording() {  
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

                // Volume needs to be 1.0 here to avoid double applying the volume
                FAudioPacketView view(packet.buffer(), packet.props(), 1.0);
                if (_encoder) {
                    _encoder->addAudioFrame(view, _encoderIdx, packet.syncTime());
                }
            });
            // Need this sleep in here to reduce contetion on the lockfree queues.
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }
    });
}

double PortaudioAudioRecorderImpl::initialVolume() const {
    return _initialVolume;
}

void PortaudioAudioRecorderImpl::setVolume(double volume) {
    _volume = volume;
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

PortaudioAudioRecorder::PortaudioAudioRecorder():
    _impl(new PortaudioAudioRecorderImpl) {

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

void PortaudioAudioRecorder::loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet) {
    _impl->loadDevice(dir, device, deviceSet);
}

double PortaudioAudioRecorder::initialVolume() const {
    return _impl->initialVolume();
}

const AudioPacketProperties& PortaudioAudioRecorder::props() const {
    return _impl->props();
}

void PortaudioAudioRecorder::setVolume(double volume) {
    _impl->setVolume(volume);
}

}