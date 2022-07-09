#include "recorder/audio/ffmpeg_audio_buffer.h"
#include "shared/audio/timing.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"

namespace service::recorder::audio {
namespace {

template<typename T>
void mergeWithRingBufferData(boost::circular_buffer<uint8_t>& outBuffer, size_t outIndex, const T* inBuffer, size_t inIndex) {
    uint8_t outRawData[sizeof(T)];
    for (auto i = 0; i < sizeof(T); ++i) {
        outRawData[i] = outBuffer[outIndex * sizeof(T) + i];
    }

    T* typedRawData = reinterpret_cast<T*>(outRawData);
    *typedRawData += inBuffer[inIndex];

    for (auto i = 0; i < sizeof(T); ++i) {
        outBuffer[outIndex * sizeof(T) + i] = outRawData[i];
    }
}

template<typename T>
void appendToRingBufferData(boost::circular_buffer<uint8_t>& outBuffer, const T* inBuffer, size_t inIndex) {
    const uint8_t* rawBuffer = reinterpret_cast<const uint8_t*>(inBuffer);
    const auto rawIndex = inIndex * sizeof(T);

    size_t overflow = 0;
    for (auto i = 0; i < sizeof(T); ++i) {
        outBuffer.push_back(rawBuffer[rawIndex+i]);
    }
}

const std::chrono::nanoseconds maxAudioDeviation = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(80));

}

FfmpegAudioBuffer::FfmpegAudioBuffer(AVSampleFormat format, int channels, int sampleRate):
    _format(format),
    _channels(channels),
    _sampleRate(sampleRate)
{
    _isPlanar = av_sample_fmt_is_planar(_format);
    if (_isPlanar) {
        _buffer.resize(channels);
    } else {
        _buffer.resize(1);
    }
    _bytesPerSample = av_get_bytes_per_sample(_format);

    for (auto& plane: _buffer) {
        // At max, we store a second worth of data in the buffer. Honestly if we need a buffer length
        // greater than a second we've fucked up.
        plane.data.set_capacity(sampleRate * _bytesPerSample * (_isPlanar ? 1 : _channels));
    }
}

void FfmpegAudioBuffer::startBuffer() {
    std::scoped_lock guard(_mutex);
    for (auto& plane: _buffer) {
        if (!plane.startTm) {
            plane.startTm = service::recorder::encoder::AVSyncClock::now();
        }
    }
}

void FfmpegAudioBuffer::addFrame(AVFrame* frame, size_t inputIndex, const service::recorder::encoder::AVSyncClock::time_point& tm) {
    std::scoped_lock guard(_mutex);
    const auto channelMultiplier = (_isPlanar ? 1 : _channels);

    for (size_t c = 0; c < _buffer.size(); ++c) {
        auto& plane = _buffer[c];
        if (!plane.startTm) {
            continue;
        }

        if (plane.data.empty()) {
            plane.startTm = tm;
        }

        const auto numSamplesInPlane = plane.data.size() / _bytesPerSample;
        const auto planeFrameSize = static_cast<size_t>(frame->nb_samples * channelMultiplier);

        // Note that I think we want to align to frame->nb_samples boundaries. Because it'd be silly to be writing data mid-frame.
        const auto timeSampleIndex = (shared::audio::timeDiffToNumSamples(_sampleRate, tm - plane.startTm.value()) + frame->nb_samples / 2) / frame->nb_samples * frame->nb_samples;
        const auto idealSampleIndex = plane.desiredNextIndex[inputIndex];

        // Note that our preference is to insert frames sequentially regardless of whether or not
        // the "time" that is given is off by a frame in either direction (e.g. we'll never go "backwards" in time
        // in terms of the samples we write out to the audio file). The only exception to this is when
        // we detect an abnormal jump in time (which we'll define as a time jump of >= 80ms) in which case
        // we add in silence to fill the gap to get to the proper time. To be able to accomplish this, we use the
        // "inputIndex", which we will use to identify which stream the input is coming from so we can store its
        // samples sequentially into the ringbuffer. If timeSampleIndex < idealSampleIndex, we can just use
        // idealSampleIndex - going back in time is silly. If timeSampleIndex > idealSampleIndex by a certain threshold,
        // only then do we insert silence.
        auto finalSampleIndex = idealSampleIndex;
        if (timeSampleIndex > idealSampleIndex) {
            const auto timeDiff = shared::audio::samplesToNsDiff(_sampleRate, timeSampleIndex - idealSampleIndex);
            if (timeDiff >= maxAudioDeviation) {
                finalSampleIndex = timeSampleIndex;
            }
        }

        const auto finalNextSampleIndex = finalSampleIndex + frame->nb_samples;

        // This would indicate we're overflowing our buffer. In that case, just prevent any audio more audio from coming in
        // and we should just end up feeding in silence to compensate.
        if (finalNextSampleIndex > plane.data.capacity()) {
            LOG_WARNING("Detected audio buffer overflow with data from audio device index: " << inputIndex << std::endl);
            break;
        }

        const auto startOverlappedIndex = finalSampleIndex * channelMultiplier;
        auto numOverlappingSamples = (startOverlappedIndex < numSamplesInPlane) ? std::min(numSamplesInPlane - startOverlappedIndex, planeFrameSize) : 0;

        // This inserts silence when startOverlappedIndex is larger than the amount of data in the buffer due to some form of audio deviation that we detected above.
        if (startOverlappedIndex > numSamplesInPlane) {
            for (auto i = numSamplesInPlane; i < startOverlappedIndex; ++i) {
                for (auto j = 0; j < _bytesPerSample; ++j) {
                    plane.data.push_back(0);
                }
            }
        }

        // Note that if this time overlaps with some time in the ring buffer, we need to do addition.
        // We only want to push_back into the ring buffer in the case where this "time" is in the future so
        // the incoming samples don't overlap with the samples already in the ring buffer.
        //
        // Note also that startOverlappedIndex is the *sample* index. The sample can be some multi-byte type (e.g. a float).
        for (size_t i = 0; i < channelMultiplier * frame->nb_samples; ++i) {
            switch (_format) {
                case AV_SAMPLE_FMT_FLT:
                case AV_SAMPLE_FMT_FLTP:
                    if (numOverlappingSamples > 0) {
                        mergeWithRingBufferData(plane.data, startOverlappedIndex + i, reinterpret_cast<const float*>(frame->data[c]), i);
                        --numOverlappingSamples;
                    } else {
                        appendToRingBufferData(plane.data, reinterpret_cast<const float*>(frame->data[c]), i);
                    }
                    break;
                default:
                    THROW_ERROR("Unsupported sample format.");
                    break;
            }
        }

        plane.desiredNextIndex[inputIndex] = finalNextSampleIndex;
    }
}

bool FfmpegAudioBuffer::hasFrameOlderThanMs(size_t samples, const std::chrono::milliseconds& ms) {
    std::scoped_lock guard(_mutex);

    // Going to also assume all channels are created equal.
    if (_buffer.empty() || !_buffer.front().startTm) {
        return false;
    }

    // The *entire* packet should be ms old.
    const auto oldestSampleTm = _buffer.front().startTm.value() + shared::audio::samplesToNsDiff(_sampleRate, samples);
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(service::recorder::encoder::AVSyncClock::now() - oldestSampleTm);

    // This should be > not >= because just in case elapsed = ms = 0.
    // In which case, we definitely don't want to be writing out frames.
    return elapsed > ms;
}

void FfmpegAudioBuffer::popFrameInto(AVFrame* output) {
    std::scoped_lock guard(_mutex);

    const auto channelMultiplier = _bytesPerSample * (_isPlanar ? 1 :  _channels);
    for (size_t c = 0; c < _buffer.size(); ++c) {
        memset(output->data[c], 0, output->nb_samples * channelMultiplier);

        auto& plane = _buffer[c];
        if (!plane.startTm) {
            continue;
        }

        const auto samplesToPop = std::min(output->nb_samples, static_cast<int>(plane.data.size()));
        if (samplesToPop > 0) {
            auto& plane = _buffer[c];
            for (size_t i = 0; i < samplesToPop * channelMultiplier; ++i) {
                output->data[c][i] = plane.data.front();
                plane.data.pop_front();
            }
        }

        for (auto& kvp: plane.desiredNextIndex) {
            if (samplesToPop >= kvp.second) {
                kvp.second = 0;
            } else {
                kvp.second -= samplesToPop;
            }
        }

        // Use the output number of samples instead of samplesToPop. In the case where
        // nb_samples > _samplesInBuffer, the next frame is going to set buffer start time anyway.
        plane.startTm = plane.startTm.value() + shared::audio::samplesToNsDiff(_sampleRate, output->nb_samples);
    }

}

}