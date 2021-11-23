#include "recorder/audio/wasapi_audio_recorder.h"
#include "recorder/audio/win32/wasapi_interface.h"
#include "shared/audio/timing.h"
#include "shared/strings/strings.h"
#include "shared/log/log.h"
#include "recorder/audio/audio_packet_view.h"
#include "recorder/audio/fixed_size_audio_packet.h"

#include <atlbase.h>
#include <audioclient.h>
#include <mmdeviceapi.h>

#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>
#include <chrono>
#include <limits>
#include <thread>
#include <type_traits>
#include <mutex>

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

namespace service::recorder::audio {
namespace {

template<typename T, std::enable_if_t<std::is_floating_point_v<T>>* = 0>
float convertSampleToFloat(T data) {
    return static_cast<float>(data);
}

template<typename T, std::enable_if_t<std::is_integral_v<T>>* = 0>
float convertSampleToFloat(T data) {
    // We use the absolute value of the min since the min value is generally larger (absoulutely) than the max value and I'd rather keep things within the [-1, 1] range.
    return static_cast<float>(data) / std::abs(std::numeric_limits<T>::min());
}

// T - the type that's represented by the passed in data.
template<typename T>
void writeAudioSampleToBuffer(const BYTE* data, float& output) {
    // This should be an array of size 1 effectively.
    const T* typedData = reinterpret_cast<const T*>(data);
    output = convertSampleToFloat(*typedData);
}

}

constexpr size_t audioSamplesPerFrame = 1024;
using AudioPacket = FixedSizeAudioPacket<float, audioSamplesPerFrame>;
using AudioPacketQueue = boost::lockfree::spsc_queue<AudioPacket, boost::lockfree::capacity<4096>, boost::lockfree::fixed_sized<true>>;
using FAudioPacketView = AudioPacketView<float>;
using SyncTime = service::recorder::encoder::AVSyncClock::time_point;

class WasapiAudioRecorderImpl {
public:
    ~WasapiAudioRecorderImpl();

    void startRecording();
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex);
    void stop();
    void loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet);
    double initialVolume() const { return _initialVolume; }
    void setVolume(double volume) { _volume = volume; }

    bool exists() const { return _exists; }
    const AudioPacketProperties& props() const { return _props; }

private:
    void printWarning(const std::string& msg, HRESULT hr) const;
    void handleData(const SyncTime& tm, const BYTE* data, uint32_t numFrames);

    EAudioDeviceDirection _dir;
    bool _exists = false;
    std::atomic<double> _volume = 1.0;
    double _initialVolume = 1.0;
    bool _mono = false;
    AudioPacketProperties _props;
    std::atomic<bool> _running = false;
    service::system::AudioDeviceSettings _inDevice;
    WAVEFORMATEXTENSIBLE _pwfx;
    AudioPacketQueue _packetQueue;

    // WASAPI recording
    CComPtr<IMMDevice> _device;
    CComPtr<IAudioClient> _audioClient;
    CComPtr<IAudioCaptureClient> _captureClient;
    double _bufferDuration = 0.0;
    std::thread _recordingThread;
    std::thread _packetThread;

    // For sending data for encoding
    service::recorder::encoder::AvEncoder* _encoder = nullptr;
    size_t _encoderIdx = 0;
    std::mutex _encoderMutex;
};

WasapiAudioRecorderImpl::~WasapiAudioRecorderImpl() {
}

void WasapiAudioRecorderImpl::printWarning(const std::string& msg, HRESULT hr) const {
    LOG_WARNING(msg << ": " << hr << " for [" << _inDevice.device << " (" << _inDevice.id << ")]" << std::endl);
}

void WasapiAudioRecorderImpl::loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet) {
    LOG_INFO("Load Selected Audio Device: '" << device.device << "(" << device.id << ")" << "' @ " << device.volume << " [Mono: " << device.mono << "]" << std::endl);
    _inDevice = device;

    bool isDefault = false;
    // The input device contains the ID - we only use the input dir if the passed in device is the default device.
    if (device.id.empty()) {
        _device = service::recorder::audio::win32::WASAPIInterface::getDefaultDevice(dir);
        isDefault = true;
    } else {
        _device = service::recorder::audio::win32::WASAPIInterface::getDeviceFromId(device.id);
    }

    if (!_device) {
        printWarning("...Failed to find audio device.", 0);
        if (!isDefault) {
            loadDevice(dir, device.createDefault(), deviceSet);
        }
        return;
    }

    std::string selectedDeviceId;
    HRESULT hr = S_OK;
    {
        LPWSTR deviceId = nullptr;
        HRESULT hr = _device->GetId(&deviceId);
        if (hr != S_OK) {
            printWarning("...Failed to get ID", hr);
            if (!isDefault) {
                loadDevice(dir, device.createDefault(), deviceSet);
            }
            return;
        }
        selectedDeviceId = shared::strings::wcsToUtf8(deviceId);
        CoTaskMemFree(deviceId);
    }

    if (deviceSet.find(selectedDeviceId) != deviceSet.end()) {
        printWarning("...Duplicate device detected. Ignoring", 0);
        if (!isDefault) {
            loadDevice(dir, device.createDefault(), deviceSet);
        }
        return;
    }

    hr = _device->Activate(
        __uuidof(IAudioClient),
        CLSCTX_ALL,
        nullptr,
        (void**)&_audioClient
    );

    if (hr != S_OK) {
        printWarning("...Failed to get audio client", hr);
        if (!isDefault) {
            loadDevice(dir, device.createDefault(), deviceSet);
        }
        return;
    }

    WAVEFORMATEX* pwfx = nullptr;
    hr = _audioClient->GetMixFormat(&pwfx);
    if (hr != S_OK) {
        printWarning("...Failed to get mix format", hr);
        if (!isDefault) {
            loadDevice(dir, device.createDefault(), deviceSet);
        }
        return;
    }
    _pwfx = *reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pwfx);
    CoTaskMemFree(pwfx);

    if (_pwfx.Format.wFormatTag != WAVE_FORMAT_EXTENSIBLE) {
        printWarning("...Unsupported format tag", pwfx->wFormatTag);
        if (!isDefault) {
            loadDevice(dir, device.createDefault(), deviceSet);
        }
        return;
    }

    // Also need to do a check to make sure the subformat is supported.
    if (_pwfx.SubFormat != KSDATAFORMAT_SUBTYPE_PCM && _pwfx.SubFormat != KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
        wchar_t guid[64] = { 0 };
        StringFromGUID2(_pwfx.SubFormat, guid, 64);

        printWarning("...Unsupported subformat: " + shared::strings::wcsToUtf8(std::wstring(guid)), 0);
        if (!isDefault) {
            loadDevice(dir, device.createDefault(), deviceSet);
        }
        return;
    }

    hr = _audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        (dir == EAudioDeviceDirection::Output) ? AUDCLNT_STREAMFLAGS_LOOPBACK : 0,
        REFTIMES_PER_SEC,
        0,
        &_pwfx.Format,
        nullptr
    );

    if (hr != S_OK) {
        printWarning("...Failed to initialize audio client", hr);
        if (!isDefault) {
            loadDevice(dir, device.createDefault(), deviceSet);
        }
        return;
    }

    uint32_t bufferFrameCount = 0;
    hr = _audioClient->GetBufferSize(&bufferFrameCount);
    if (hr != S_OK) {
        printWarning("...Failed to get buffer size", hr);
        if (!isDefault) {
            loadDevice(dir, device.createDefault(), deviceSet);
        }
        return;
    }

    hr = _audioClient->GetService(
        __uuidof(IAudioCaptureClient),
        (void**)&_captureClient
    );
    if (hr != S_OK) {
        printWarning("...Failed to get capture client", hr);
        if (!isDefault) {
            loadDevice(dir, device.createDefault(), deviceSet);
        }
        return;
    }

    _exists = true;
    _volume = device.volume;
    _initialVolume = device.volume;
    _mono = device.mono;
    _bufferDuration = static_cast<double>(REFTIMES_PER_SEC) * bufferFrameCount / _pwfx.Format.nSamplesPerSec;

    // We set the props that we want here. The only thing we want to keep consistent
    // is the sample rate so we don't have to resample ourselves. Number of channels we keep
    // consistent too and let the encoder go down to whatever number of channels it wants to encode.
    _props.samplingRate = _pwfx.Format.nSamplesPerSec;
    _props.numChannels = device.mono ? 1 : _pwfx.Format.nChannels;
    // Will be filled in on a per-packet basis.
    _props.numSamples = 0;
    _props.forceMono = device.mono;
    _props.isPlanar = false;

    deviceSet.insert(selectedDeviceId);
}

void WasapiAudioRecorderImpl::startRecording() {
    _running = true;
    _packetThread = std::thread([this](){
        while (_running) {
            _packetQueue.consume_all([this](const AudioPacket& packet){
                std::lock_guard<std::mutex> guard(_encoderMutex);

                FAudioPacketView view(packet.buffer(), packet.props(), _volume);
                if (_encoder) {
                    _encoder->addAudioFrame(view, _encoderIdx, packet.syncTime());
                }
            });
            // Need this sleep in here to reduce contetion on the lockfree queues.
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }
    });


    _recordingThread = std::thread([this](){
        HRESULT hr = _audioClient->Start();
        if (hr != S_OK) {
            printWarning("...Failed to start audio client", hr);
            return;
        }

        while (_running) {
            auto packetTime = service::recorder::encoder::AVSyncClock::now();
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int64_t>(_bufferDuration/REFTIMES_PER_MILLISEC/2.0)));

            uint32_t packetLength = 0;
            hr = _captureClient->GetNextPacketSize(&packetLength);
            if (hr != S_OK) {
                printWarning("...Failed to get next packet size", hr);
                break;
            }

            while (packetLength != 0) {
                uint32_t numFramesAvailable = 0;
                DWORD flags = 0;
                BYTE* pData = nullptr;

                hr = _captureClient->GetBuffer(&pData, &numFramesAvailable, &flags, nullptr, nullptr);
                if (hr != S_OK) {
                    printWarning("...Failed to get buffer", hr);
                    break;
                }

                if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                    pData = nullptr;
                }

                // Create a packet and send it to the packet queue.
                // We don't want to send the packet directly to the encoder here since
                // that may involve locks that we don't want in this inner loop.
                handleData(packetTime, pData, numFramesAvailable);

                hr = _captureClient->ReleaseBuffer(numFramesAvailable);
                if (hr != S_OK) {
                    printWarning("...Failed to release buffer", hr);
                    break;
                }

                hr = _captureClient->GetNextPacketSize(&packetLength);
                if (hr != S_OK) {
                    printWarning("...Failed to get next packet size", hr);
                    break;
                }

                // Need to increment the packet time by an appropriate amount.
                // The default is to assume that we're getting data properly so N frames can be divided by the sample rate
                // to get the number of elapsed seconds.
                const auto elapsedNs = static_cast<double>(numFramesAvailable) / _props.samplingRate * 1e9;
                packetTime += std::chrono::nanoseconds(static_cast<size_t>(elapsedNs));
            }
        }
    });
}

void WasapiAudioRecorderImpl::handleData(const SyncTime& tm, const BYTE* data, uint32_t numFrames) {
    // At this point we need to wrangle the incoming data which should be in the format specified by WAVEFORMATEXTENSIBLE
    // into the format we want specified by the AudioPacketProperties. Note that a frame contains all the samples for each
    // channel within it - thus, the incoming data is interweaved. Note that we may need to create multiple packets from the
    // incoming data if the number of frames is greater than audioSamplesPerFrame.
    size_t framesRead = 0;
    while (numFrames > 0) {
        auto props = _props;

        // Note that audioSamplesPerFrame is literally how many samples across ALL channels we can pack into a packet.
        props.numSamples = std::min(static_cast<size_t>(numFrames), audioSamplesPerFrame / props.numChannels);
        props.numChannels = _pwfx.Format.nChannels;

        // Our process of getting this data into an audio packet is two fold - mainly just to make sure we maximize re-usage of
        // code. If this turns out to be a performance issue we can re-evaluate.
        //  1) Create an audio packet that has the same number of channels as the incoming data - the only thing we're doing here is a
        //     type conversion from whateve the fuck input data type (8-bit/16-bit PCM, or 32-bit/64-bit floating point).
        //  2) If the desired data is supposed to be mono, then we create a separate audio packet and use the packet from #1 as a view.
        AudioPacket packet(props, tm);

        const auto indexStart = framesRead * _pwfx.Format.nChannels * _pwfx.Format.wBitsPerSample/8;
        const BYTE* subData = &data[indexStart];
        size_t outIndex = 0;
        for (auto i = 0; i < props.numSamples; ++i) {
            for (auto c = 0; c < props.numChannels; ++c) {
                const auto sampleIndex = (i * props.numChannels + c) * _pwfx.Format.wBitsPerSample/8;
                const BYTE* sampleData = &subData[sampleIndex];

                if (_pwfx.SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
                    if (_pwfx.Format.wBitsPerSample == 8) {
                        writeAudioSampleToBuffer<int8_t>(sampleData, packet.buffer()[outIndex]);
                    } else if (_pwfx.Format.wBitsPerSample == 16) {
                        writeAudioSampleToBuffer<int16_t>(sampleData, packet.buffer()[outIndex]);
                    }
                } else if (_pwfx.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
                    if (_pwfx.Format.wBitsPerSample == 32) {
                        writeAudioSampleToBuffer<float>(sampleData, packet.buffer()[outIndex]);
                    } else if (_pwfx.Format.wBitsPerSample == 64) {
                        writeAudioSampleToBuffer<double>(sampleData, packet.buffer()[outIndex]);
                    }
                }
                outIndex++;
            }
        }

        if (props.forceMono && props.numChannels > 1) {
            auto monoProps = props;
            props.numChannels = 1;

            // Don't apply volume here - that'll get applied when we actually go to encode.
            FAudioPacketView view(packet.buffer(), packet.props(), 1.0);
            AudioPacket monoPacket(monoProps, packet.syncTime());
            monoPacket.copyFrom(view, 0, props.numSamples);
            _packetQueue.push(monoPacket);
        } else {
            _packetQueue.push(packet);
        }

        numFrames -= static_cast<uint32_t>(props.numSamples);
        framesRead += props.numSamples;
    }
}

void WasapiAudioRecorderImpl::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) {
    std::lock_guard<std::mutex> guard(_encoderMutex);
    _encoder = encoder;
    _encoderIdx = encoderIndex;
}

void WasapiAudioRecorderImpl::stop() {
    _running = false;
    if (_packetThread.joinable()) {
        _packetThread.join();
    }

    if (_recordingThread.joinable()) {
        _recordingThread.join();
    }
    HRESULT hr = _audioClient->Stop();
    if (hr != S_OK) {
        LOG_WARNING("...Failed to stop audio client: " << hr << std::endl);
    }
}

WasapiAudioRecorder::WasapiAudioRecorder():
    _impl(new WasapiAudioRecorderImpl)
{
}

WasapiAudioRecorder::~WasapiAudioRecorder() {

}

void WasapiAudioRecorder::startRecording() {
    _impl->startRecording();
}

void WasapiAudioRecorder::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) {
    _impl->setActiveEncoder(encoder, encoderIndex);
}

void WasapiAudioRecorder::stop() {
    _impl->stop();
}

void WasapiAudioRecorder::loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet) {
    _impl->loadDevice(dir, device, deviceSet);
}

double WasapiAudioRecorder::initialVolume() const {
    return _impl->initialVolume();
}

void WasapiAudioRecorder::setVolume(double volume) {
    _impl->setVolume(volume);
}

bool WasapiAudioRecorder::exists() const {
    return _impl->exists();
}

const AudioPacketProperties& WasapiAudioRecorder::props() const {
    return _impl->props();
}

}