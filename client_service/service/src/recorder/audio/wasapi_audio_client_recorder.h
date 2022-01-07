#pragma once

#include "recorder/audio/audio_recorder.h"
#include "recorder/audio/audio_packet_view.h"
#include "recorder/audio/fixed_size_audio_packet.h"

#include <wil/com.h>
#include <audioclient.h>

#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>
#include <memory>

namespace service::recorder::audio {

// The internal class used by WasapiAudioRecorder and WasapiProgramRecorder
// to start recording when given IAudioClient.
class WasapiAudioClientRecorder {
    using AudioPacket = FixedSizeAudioPacket<float, 1024>;
    using AudioPacketQueue = boost::lockfree::spsc_queue<AudioPacket, boost::lockfree::capacity<4096>, boost::lockfree::fixed_sized<true>>;
    using FAudioPacketView = AudioPacketView<float>;
    using SyncTime = service::recorder::encoder::AVSyncClock::time_point;

public:
    WasapiAudioClientRecorder(wil::com_ptr<IAudioClient> client, const std::string& context, bool mono, bool isLoopback);

    void startRecording();
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex);
    void stop();

    void setVolume(double volume) { _volume = volume; }
    bool exists() const { return _exists; }
    const AudioPacketProperties& props() const { return _props; }

private:
    void printWarning(const std::string& msg, HRESULT hr) const;
    void handleData(const SyncTime& tm, const BYTE* data, uint32_t numFrames);

    wil::com_ptr<IAudioClient> _audioClient;
    std::string _context;
    std::atomic<bool> _running = false;

    wil::com_ptr<IAudioCaptureClient> _captureClient;
    double _bufferDuration = 0.0;

    // For sending data for encoding
    service::recorder::encoder::AvEncoder* _encoder = nullptr;
    size_t _encoderIdx = 0;
    std::mutex _encoderMutex;
    std::atomic<double> _volume = 1.0;
    bool _exists = false;
    AudioPacketProperties _props;
    WAVEFORMATEXTENSIBLE _pwfx;
    AudioPacketQueue _packetQueue;

    std::thread _recordingThread;
    std::thread _packetThread;
};

using WasapiAudioClientRecorderPtr = std::unique_ptr<WasapiAudioClientRecorder>;

}