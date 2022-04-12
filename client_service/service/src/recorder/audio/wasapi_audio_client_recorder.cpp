#include "recorder/audio/wasapi_audio_client_recorder.h"
#include "shared/strings/strings.h"
#include "shared/log/log.h"
#include "recorder/audio/win32/wasapi_interface.h"

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

WasapiAudioClientRecorder::WasapiAudioClientRecorder(wil::com_ptr<IAudioClient> client, const std::string& context, bool mono, bool isLoopback):
    _audioClient(client),
    _context(context)
{
    LOG_INFO("Loading WASAPI Audio Client Recorder: " << context << std::endl);
    WAVEFORMATEX* pwfx = nullptr;
    HRESULT hr = _audioClient->GetMixFormat(&pwfx);
    if (hr != S_OK) {
        printWarning("...Failed to get mix format [attempting to use default output endpoint wave format]", hr);

        const auto defaultDevice = service::recorder::audio::win32::WASAPIInterface::getDefaultDevice(service::recorder::audio::EAudioDeviceDirection::Output);
        wil::com_ptr<IAudioClient> defaultAudioClient;
        hr = defaultDevice->Activate(
            __uuidof(IAudioClient),
            CLSCTX_ALL,
            nullptr,
            (void**)&defaultAudioClient
        );

        if (hr != S_OK) {
            printWarning("...Failed to get audio client when getting default mix", hr);
            return;
        }

        hr = defaultAudioClient->GetMixFormat(&pwfx);
        if (hr != S_OK) {
            printWarning("...Failed to get default mix format [aborting attempt]", hr);
            return;
        }
    };

    _pwfx = *pwfx;

    if (_pwfx.wFormatTag != WAVE_FORMAT_EXTENSIBLE &&
        _pwfx.wFormatTag != WAVE_FORMAT_IEEE_FLOAT &&
        _pwfx.wFormatTag != WAVE_FORMAT_PCM) {
        printWarning("...Unsupported format tag", pwfx->wFormatTag);
        CoTaskMemFree(pwfx);
        return;
    }

    if (_pwfx.wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        _ewfx = *reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pwfx);
        // Also need to do a check to make sure the subformat is supported.
        if (_ewfx.SubFormat != KSDATAFORMAT_SUBTYPE_PCM && _ewfx.SubFormat != KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
            wchar_t guid[64] = { 0 };
            StringFromGUID2(_ewfx.SubFormat, guid, 64);

            printWarning("...Unsupported subformat: " + shared::strings::wcsToUtf8(std::wstring(guid)), 0);
            CoTaskMemFree(pwfx);
            return;
        }
    }

    CoTaskMemFree(pwfx);

    // Also check bits per sample. PCM supports 8 and 16, Float supports 32 and 64.
    if (isPcm()) {
        if (_pwfx.wBitsPerSample != 8 && _pwfx.wBitsPerSample != 16) {
            printWarning("...Unsupported number of bytes for PCM: " + std::to_string(_pwfx.wBitsPerSample), 0);
            return;
        }
    } else if (isFloat()) {
        if (_pwfx.wBitsPerSample != 32 && _pwfx.wBitsPerSample != 64) {
            printWarning("...Unsupported number of bytes for float: " + std::to_string(_pwfx.wBitsPerSample), 0);
            return;
        }
    }

    LOG_INFO("...Retrieving device period." << std::endl);
    REFERENCE_TIME requestedDuration = 0;
    hr = _audioClient->GetDevicePeriod(nullptr, &requestedDuration);
    if (hr != S_OK) {
        printWarning("...Failed to get device period...using default" , hr);
        requestedDuration = REFTIMES_PER_SEC;
    }

    LOG_INFO("...Initializing WASAPU Audio Client: " << context << std::endl); 
    hr = _audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        (isLoopback ? AUDCLNT_STREAMFLAGS_LOOPBACK : 0) | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        requestedDuration,
        0,
        (_pwfx.wFormatTag == WAVE_FORMAT_EXTENSIBLE) ? &_ewfx.Format : &_pwfx,
        nullptr
    );

    if (hr != S_OK) {
        printWarning("...Failed to initialize audio client", hr);
        return;
    }

    // We need to create the event that will let us wait until MS tells us
    // there's an audio buffer available.
    _eventCallback = CreateEvent(NULL, false, false, NULL);
    if (!_eventCallback) {
        printWarning("...Failed to create event callback", E_FAIL);
        return;
    }

    hr = _audioClient->SetEventHandle(_eventCallback);
    if (hr != S_OK) {
        printWarning("...Failed to set event handle", hr);
        return;
    }

    hr = _audioClient->GetService(
        __uuidof(IAudioCaptureClient),
        (void**)&_captureClient
    );

    if (hr != S_OK) {
        printWarning("...Failed to get capture client", hr);
        return;
    }

    _exists = true;
    LOG_INFO("Success Loading WASAPI [" << context << "]: " << _pwfx.nSamplesPerSec << " (Device Samples/Sec) -- " << _pwfx.nChannels << " (Device Channels)" << std::endl);

    // We set the props that we want here. The only thing we want to keep consistent
    // is the sample rate so we don't have to resample ourselves. Number of channels we keep
    // consistent too and let the encoder go down to whatever number of channels it wants to encode.
    _props.samplingRate = _pwfx.nSamplesPerSec;
    _props.numChannels = mono ? 1 : _pwfx.nChannels;
    // Will be filled in on a per-packet basis.
    _props.numSamples = 0;
    _props.forceMono = mono;
    _props.isPlanar = false;
}

WasapiAudioClientRecorder::~WasapiAudioClientRecorder() {
    if (_eventCallback) {
        CloseHandle(_eventCallback);
    }
}

void WasapiAudioClientRecorder::printWarning(const std::string& msg, HRESULT hr) const {
    LOG_WARNING(msg << ": " << hr << " for [" << _context << "]" << std::endl);
}

void WasapiAudioClientRecorder::handleData(const SyncTime& tm, const BYTE* data, uint32_t numFrames) {
    // At this point we need to wrangle the incoming data which should be in the format specified by WAVEFORMATEXTENSIBLE
    // into the format we want specified by the AudioPacketProperties. Note that a frame contains all the samples for each
    // channel within it - thus, the incoming data is interweaved. Note that we may need to create multiple packets from the
    // incoming data if the number of frames is greater than audioSamplesPerFrame.
    size_t framesRead = 0;
    while (numFrames > 0) {
        auto props = _props;

        // Note that audioSamplesPerFrame is literally how many samples across ALL channels we can pack into a packet.
        props.numSamples = std::min(static_cast<size_t>(numFrames), AudioPacket::N / props.numChannels);
        props.numChannels = _pwfx.nChannels;

        // Our process of getting this data into an audio packet is two fold - mainly just to make sure we maximize re-usage of
        // code. If this turns out to be a performance issue we can re-evaluate.
        //  1) Create an audio packet that has the same number of channels as the incoming data - the only thing we're doing here is a
        //     type conversion from whateve the fuck input data type (8-bit/16-bit PCM, or 32-bit/64-bit floating point).
        //  2) If the desired data is supposed to be mono, then we create a separate audio packet and use the packet from #1 as a view.
        AudioPacket packet(props, tm);

        const auto indexStart = framesRead * _pwfx.nChannels * _pwfx.wBitsPerSample/8;
        const BYTE* subData = &data[indexStart];
        size_t outIndex = 0;
        for (auto i = 0; i < props.numSamples; ++i) {
            for (auto c = 0; c < props.numChannels; ++c) {
                const auto sampleIndex = (i * props.numChannels + c) * _pwfx.wBitsPerSample/8;
                const BYTE* sampleData = &subData[sampleIndex];

                if (isPcm()) {
                    if (_pwfx.wBitsPerSample == 8) {
                        writeAudioSampleToBuffer<int8_t>(sampleData, packet.buffer()[outIndex]);
                    } else if (_pwfx.wBitsPerSample == 16) {
                        writeAudioSampleToBuffer<int16_t>(sampleData, packet.buffer()[outIndex]);
                    }
                } else if (isFloat()) {
                    if (_pwfx.wBitsPerSample == 32) {
                        writeAudioSampleToBuffer<float>(sampleData, packet.buffer()[outIndex]);
                    } else if (_pwfx.wBitsPerSample == 64) {
                        writeAudioSampleToBuffer<double>(sampleData, packet.buffer()[outIndex]);
                    }
                }
                outIndex++;
            }
        }

        if (props.forceMono && props.numChannels > 1) {
            auto monoProps = props;
            monoProps.numChannels = 1;

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

bool WasapiAudioClientRecorder::isPcm() const {
    return _pwfx.wFormatTag == WAVE_FORMAT_PCM || (_pwfx.wFormatTag == WAVE_FORMAT_EXTENSIBLE && _ewfx.SubFormat == KSDATAFORMAT_SUBTYPE_PCM);
}

bool WasapiAudioClientRecorder::isFloat() const {
    return _pwfx.wFormatTag == WAVE_FORMAT_IEEE_FLOAT || (_pwfx.wFormatTag == WAVE_FORMAT_EXTENSIBLE && _ewfx.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
}

void WasapiAudioClientRecorder::startRecording() {
    if (!exists()) {
        return;
    }

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
            DWORD retval = WaitForSingleObject(_eventCallback, 2000);
            if (retval != WAIT_OBJECT_0) {
                printWarning("...Timed out in waiting for callback", E_FAIL);
                continue;
            }

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

void WasapiAudioClientRecorder::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) {
    std::lock_guard<std::mutex> guard(_encoderMutex);
    _encoder = encoder;
    _encoderIdx = encoderIndex;
}

void WasapiAudioClientRecorder::stop() {
    _running = false;
    LOG_INFO("...Stopping Audio Client Recorder [Packet Thread]" << std::endl);
    if (_packetThread.joinable()) {
        _packetThread.join();
    }

    LOG_INFO("...Stopping Audio Client Recorder [Recording Thread]" << std::endl);
    if (_recordingThread.joinable()) {
        _recordingThread.join();
    }

    LOG_INFO("...Stopping Audio Client Recorder [Audio Client]" << std::endl);
    HRESULT hr = _audioClient->Stop();
    if (hr != S_OK) {
        printWarning("...Failed to stop audio client", hr);
    }

    LOG_INFO("...Finish Audio Client Recorder Stop" << std::endl);
}

}