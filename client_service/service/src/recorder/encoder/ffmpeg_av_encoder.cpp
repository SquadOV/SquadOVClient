#include "recorder/encoder/ffmpeg_av_encoder.h"

#include "recorder/audio/fixed_size_audio_packet.h"
#include "shared/env.h"
#include "shared/errors/error.h"
#include "shared/log/log.h"
#include "shared/timer.h"
#include "shared/filesystem/utility.h"

#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>
#include <cstdio>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <vector>

extern "C" {
#include <libavutil/audio_fifo.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libavutil/mathematics.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include "recorder/encoder/ffmpeg_utils.h"
}

#define LOG_FRAME_TIME 0

using namespace std::chrono_literals;

namespace service::recorder::encoder {
namespace {

int64_t packetPropsToFFmpegChannelLayout(const service::recorder::audio::AudioPacketProperties& props) {
    return (props.numChannels == 2) ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO;
}

AVSampleFormat packetPropsToFFmpegSampleFmt(const service::recorder::audio::AudioPacketProperties& props) {
    return props.isPlanar ? AV_SAMPLE_FMT_FLTP : AV_SAMPLE_FMT_FLT;
}

int packetPropsToFFmpegSampleRate(const service::recorder::audio::AudioPacketProperties& props) {
    return static_cast<int>(props.samplingRate);
}

constexpr size_t audioNumChannels = 2;
constexpr size_t audioSamplesPerFrame = 480;

void encode(AVCodecContext* cctx, AVFormatContext* fctx, AVFrame* frame, AVStream* st, int64_t ptsOffset, int64_t dtsOffset){
    // Encode video frame.
    auto ret = avcodec_send_frame(cctx, frame);
    if (ret < 0) {
        THROW_ERROR("Failed to send  frame: " << ret);
    }

    while (true) {
        AVPacket packet = { 0 };
        const auto ret = avcodec_receive_packet(cctx, &packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            THROW_ERROR("Failed to receive video packet.");
        }

        // Force a duration here - not sure why this is necessary but if we don't set this
        // here then the duration of the video will be one frame short. I think that's
        // causing some audio/video drfit. PEPE HANDS.
        packet.duration = 1;

        av_packet_rescale_ts(&packet, cctx->time_base, st->time_base);
        packet.pts += ptsOffset;
        packet.dts += dtsOffset;
        packet.stream_index = st->index;

        if (av_interleaved_write_frame(fctx, &packet) < 0) {
            THROW_ERROR("Failed to write packet");
        }
        av_packet_unref(&packet);
    }
}

}

class FfmpegAvEncoderImpl {
public:
    explicit FfmpegAvEncoderImpl(const std::string& streamUrl);
    ~FfmpegAvEncoderImpl();

    const std::string& streamUrl() const { return _streamUrl; }
    void initializeVideoStream(size_t fps, size_t width, size_t height);
    void addVideoFrame(const service::recorder::image::Image& frame);
    void getVideoDimensions(size_t& width, size_t& height);

    void initializeAudioStream();
    size_t addAudioInput(const service::recorder::audio::AudioPacketProperties& inputProps);
    void addAudioFrame(const service::recorder::audio::FAudioPacketView& view, size_t encoderIdx, const AVSyncClock::time_point& tm);

    void start();
    void stop();
    void pauseInputRecorderProcessing(bool pause);
    void appendFromVideoFile(const std::filesystem::path& vodPath, int64_t startMs);
    shared::squadov::VodMetadata getMetadata() const;
private:
    void swapBuffers();

    AVFrame* createAudioFrame() const;
    bool hasAudioFrameAvailable() const;

    std::string _streamUrl;

    // AV Output
    AVOutputFormat* _avformat = nullptr;
    AVFormatContext* _avcontext = nullptr;
    std::mutex _encodeMutex;
    int64_t _dtsOffset = 0;
    int64_t _ptsOffset = 0;
    
    // Video Output
    SwsContext* _sws = nullptr;
    SwsContext* _nextSws = nullptr;

    AVStream* _vstream = nullptr;
    const AVCodec* _vcodec = nullptr;
    AVCodecContext* _vcodecContext = nullptr;
    AVFrame* _vframe = nullptr;
    int64_t _vFrameNum = 0;

    // Audio Output
    const AVCodec* _acodec = nullptr;
    AVCodecContext* _acodecContext = nullptr;
    AVStream* _astream = nullptr;
    AVFrame* _aframe = nullptr;
    // We need to use a tmp frame to read from the FIFO queue for each stream input.
    AVFrame* _aTmpFrame = nullptr;
    int64_t _aFrameNum = 0;

    struct AudioStreamData {
        // *Output* packet queue. Once we process the input
        // packet, we'll convert it into whatever format
        // is needed for output and store it in this queue.
        // Once the queue reaches the size of an actual frame,
        // we'll send that to the encoder to be written out.
        AVAudioFifo* fifo = nullptr;

        // samplesStorage is the place where we'll use swr to convert the
        // input audio packet into the output format.
        uint8_t** samplesStorage = nullptr;
        int sampleLineSize = 0;
        int64_t maxSamples = 0;

        // Debug Info
        uint64_t receivedSamples = 0;
        uint64_t processedSamples = 0;
        uint64_t convertedSamples = 0;
        uint64_t encodedSamples = 0;

        SwrContext* swr = nullptr;

        AudioStreamData() = default;
        AudioStreamData(const AudioStreamData&) = delete;
        AudioStreamData(AudioStreamData&&) = delete;

        void reinitSampleStorage(int inputSamples);
        void writeStoredSamplesToFifo(int numSamples);

        ~AudioStreamData();
    };
    using AudioStreamDataPtr = std::unique_ptr<AudioStreamData>;
    std::vector<AudioStreamDataPtr> _astreams;

    std::mutex _pauseMutex;
    std::condition_variable _pauseCv;
    bool _pauseInput = false;

    // Front and back buffering to support constant frame rate videos.
    std::thread _videoEncodingThread;
    bool _running = false;
    std::chrono::nanoseconds _nsPerFrame;

    std::unique_ptr<service::recorder::image::Image> _frontBuffer;

    std::mutex _backMutex;
    std::unique_ptr<service::recorder::image::Image> _backBuffer;
    bool _backBufferDirty = false;

    uint64_t _receivedVideoFrames = 0;
    uint64_t _processedVideoFrames = 0;
};

FfmpegAvEncoderImpl::AudioStreamData::~AudioStreamData() {
    av_freep(&samplesStorage[0]);
    av_audio_fifo_free(fifo);
    swr_free(&swr);
}

void FfmpegAvEncoderImpl::AudioStreamData::reinitSampleStorage(int inputSamples) {
    int64_t outputSampleRate = 0;
    AVSampleFormat outputFormat;
    int64_t outputChannelLayout = 0;
    av_opt_get_int(swr, "out_sample_rate", 0, &outputSampleRate);
    av_opt_get_sample_fmt(swr, "out_sample_fmt", 0, &outputFormat);
    av_opt_get_channel_layout(swr, "out_channel_layout", 0, &outputChannelLayout);

    int64_t inputSampleRate = 0;
    av_opt_get_int(swr, "in_sample_rate", 0, &inputSampleRate);

    const int64_t newMaxSamples = 
        (inputSamples > 0) ? 
            av_rescale_rnd(inputSamples, outputSampleRate, inputSampleRate, AV_ROUND_UP): 
            static_cast<int64_t>(audioSamplesPerFrame);

    if (newMaxSamples < maxSamples) {
        return;
    }

    maxSamples = newMaxSamples;
    if (samplesStorage) {
        av_freep(&samplesStorage[0]);
        samplesStorage = nullptr;
    }

    if (av_samples_alloc_array_and_samples(
        &samplesStorage,
        &sampleLineSize,
        av_get_channel_layout_nb_channels(static_cast<uint64_t>(outputChannelLayout)),
        static_cast<int>(maxSamples),
        outputFormat, 0) < 0) {
        THROW_ERROR("Failed to allocate converted samples.");
    }
}

void FfmpegAvEncoderImpl::AudioStreamData::writeStoredSamplesToFifo(int numSamples) {
    if (av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + numSamples) < 0) {
        THROW_ERROR("Failed to reallocate FIFO queue.");
    }

    if (av_audio_fifo_write(fifo, (void**)samplesStorage, numSamples) < numSamples) {
        THROW_ERROR("Failed to write to FIFO queue.");
    }
}

FfmpegAvEncoderImpl::FfmpegAvEncoderImpl(const std::string& streamUrl):
    _streamUrl(streamUrl) {

    _avformat = av_guess_format("mp4", nullptr, nullptr);
    if (!_avformat) {
        THROW_ERROR("Failed to find the MP4 format.");
    }

    if (avformat_alloc_output_context2(&_avcontext, _avformat, nullptr, nullptr) < 0) {
        THROW_ERROR("Failed to allocate AV context.");
    }
}

FfmpegAvEncoderImpl::~FfmpegAvEncoderImpl() {
    sws_freeContext(_sws);
    sws_freeContext(_nextSws);
    av_frame_free(&_vframe);
    avcodec_free_context(&_vcodecContext);
    av_frame_free(&_aframe);
    av_frame_free(&_aTmpFrame);
    avcodec_free_context(&_acodecContext);

    if (_avcontext->pb) {
        avio_flush(_avcontext->pb);
        avio_closep(&_avcontext->pb);
    }
    avformat_free_context(_avcontext);
}

void FfmpegAvEncoderImpl::getVideoDimensions(size_t& width, size_t& height) {
    if (!_vframe) {
        width = 0;
        height = 0;
        return;
    }

    width = static_cast<size_t>(_vframe->width);
    height = static_cast<size_t>(_vframe->height);
}

void FfmpegAvEncoderImpl::initializeVideoStream(size_t fps, size_t width, size_t height) {
    // Try to use hardware encoding first. If not fall back on mpeg4.
    const std::string encodersToUse[] = {
        "h264_amf",
        "h264_nvenc",
        "libopenh264"
    };

    bool foundEncoder = false;
    for (const auto& enc : encodersToUse) {
        try {
            _vcodec = avcodec_find_encoder_by_name(enc.c_str());
            if (!_vcodec) {
                THROW_ERROR("Failed to find the video codec.");
            }
            
            _vcodecContext = avcodec_alloc_context3(_vcodec);
            if (!_vcodecContext) {
                THROW_ERROR("Failed to allocate video context.");
            }

            _vcodecContext->width = static_cast<int>(width);
            _vcodecContext->height = static_cast<int>(height);
            _vcodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
            _vcodecContext->bit_rate = 6000000;
            _vcodecContext->thread_count = 0;

            // We're going to specify time in ms (instead of frames) so set the timebase to 1ms.
            _vcodecContext->time_base = AVRational{1, static_cast<int>(fps)};
            _vcodecContext->framerate = AVRational{static_cast<int>(fps), 1};

            // I think we get better video quality with a larger GOP size? Not sure tbh. I saw some flickering in certain formats
            // if the GOP size is too small.
            _vcodecContext->gop_size = static_cast<int>(fps * 5);
            _vcodecContext->max_b_frames = 1;

            if (_avcontext->oformat->flags & AVFMT_GLOBALHEADER) {
                _vcodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            }
            
            // Actually initialize the codec context.
            AVDictionary *options = nullptr;

            if (_vcodecContext->codec_id == AV_CODEC_ID_H264) {
                av_dict_set(&options, "preset", "medium", 0);
            }

            const auto ret = avcodec_open2(_vcodecContext, _vcodec, &options);
            if (ret < 0) {
                THROW_ERROR("Failed to initialize video context");
            }
            av_dict_free(&options);

            foundEncoder = true;
            LOG_INFO("FFmpeg Found Encoder: " << enc << std::endl);
            break;
        } catch (...) {
            avcodec_free_context(&_vcodecContext);
        }
    }

    if (!foundEncoder) {
        THROW_ERROR("Failed to find a valid encoder.");
    }

    _vstream = avformat_new_stream(_avcontext, _vcodec);
    if (!_vstream) {
        THROW_ERROR("Failed to create video stream.");
    }

    _vstream->time_base = _vcodecContext->time_base;
    avcodec_parameters_from_context(_vstream->codecpar, _vcodecContext);

    // Preallocate frame for output.
    _vframe = av_frame_alloc();
    if (!_vframe) {
        THROW_ERROR("Failed to allocate video frame");
    }

    _vframe->format = _vcodecContext->pix_fmt;
    _vframe->width = _vcodecContext->width;
    _vframe->height = _vcodecContext->height;
    _vframe->pts = 0;

    if (av_frame_get_buffer(_vframe, 0) < 0) {
        THROW_ERROR("Failed to get frame buffer.");
    }
    
    // BGRA images for the front/back buffer.
    _frontBuffer.reset(new service::recorder::image::Image());
    _backBuffer.reset(new service::recorder::image::Image());

    _nsPerFrame = std::chrono::nanoseconds(static_cast<size_t>(1.0 / fps * 1.0e+9));
}

void FfmpegAvEncoderImpl::initializeAudioStream() {
    _acodec = avcodec_find_encoder_by_name("aac");
    if (!_acodec) {
        THROW_ERROR("Failed to find the audio codec.");
    }

    _acodecContext = avcodec_alloc_context3(_acodec);
    if (!_acodecContext) {
        THROW_ERROR("Failed to allocate audio codec context.");
    }

    constexpr int sampleRate = 48000;
    _acodecContext->bit_rate = 128000; // probably good enough?
    _acodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
    _acodecContext->sample_rate = sampleRate;
    _acodecContext->channels = static_cast<int>(audioNumChannels);
    _acodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
    _acodecContext->time_base = AVRational{1, sampleRate};
    _acodecContext->frame_size = audioSamplesPerFrame;
    if (_avcontext->oformat->flags & AVFMT_GLOBALHEADER) {
        _acodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(_acodecContext, _acodec, nullptr) < 0) {
        THROW_ERROR("Failed to initialize audio context");
    }

    _astream = avformat_new_stream(_avcontext, nullptr);
    if (!_vstream) {
        THROW_ERROR("Failed to create audio stream.");
    }
    _astream->id = _avcontext->nb_streams - 1;
    _astream->time_base = _acodecContext->time_base;
    avcodec_parameters_from_context(_astream->codecpar, _acodecContext);

    _aframe = createAudioFrame();
    _aTmpFrame = createAudioFrame();
}

AVFrame* FfmpegAvEncoderImpl::createAudioFrame() const {
    AVFrame* frame = av_frame_alloc();
    frame->nb_samples = _acodecContext->frame_size;
    frame->format = _acodecContext->sample_fmt;
    frame->channel_layout = _acodecContext->channel_layout;
    frame->channels = _acodecContext->channels;
    frame->sample_rate = _acodecContext->sample_rate;
    frame->pts = 0;

    if (av_frame_get_buffer(frame, 0) < 0) {
        THROW_ERROR("Failed to get audio buffer.");
    }
    return frame;
}

size_t FfmpegAvEncoderImpl::addAudioInput(const service::recorder::audio::AudioPacketProperties& inputProps) {
    AudioStreamDataPtr streamDataPtr = std::make_unique<AudioStreamData>();
    AudioStreamData& streamData = *streamDataPtr;

    streamData.swr = swr_alloc_set_opts(nullptr,
        _acodecContext->channel_layout, // out_ch_layout
        _acodecContext->sample_fmt, // out_sample_fmt
        _acodecContext->sample_rate, // out_sample_rate
        packetPropsToFFmpegChannelLayout(inputProps), // in_ch_layout
        packetPropsToFFmpegSampleFmt(inputProps), // in_sample_fmt
        packetPropsToFFmpegSampleRate(inputProps), // in_sample_rate,
        0, // log_offset,
        nullptr // log_ctx
    );
    if (!streamData.swr) {
        THROW_ERROR("Failed to alloc swr and set options.");
    }

    if (swr_init(streamData.swr) < 0) {
        THROW_ERROR("Failed to init swr.");
    }

    // Create samples array.
    streamData.reinitSampleStorage(static_cast<int>(inputProps.numSamples));

    // Create fifo queue. 2x the frame size should be plenty
    streamData.fifo = av_audio_fifo_alloc(
        _acodecContext->sample_fmt,
        _acodecContext->channels,
        _acodecContext->frame_size * 2
    );

    if (!streamData.fifo) {
        THROW_ERROR("Failed to allocate fifo queue.");
    }

    _astreams.emplace_back(std::move(streamDataPtr));
    return _astreams.size() - 1;
}

void FfmpegAvEncoderImpl::addVideoFrame(const service::recorder::image::Image& frame) {
    if (!_running) {
        return;
    }

    std::unique_lock<std::mutex> guard(_backMutex);

    if (_backBuffer->width() != frame.width() || _backBuffer->height() != frame.height()) {
        _backBuffer->initializeImage(frame.width(), frame.height());

        // SWS for scaling the buffer into YUV420p for encoding.
        if (!!_nextSws) {
            sws_freeContext(_nextSws);
        }
        _nextSws = sws_getContext(
            static_cast<int>(_backBuffer->width()),
            static_cast<int>(_backBuffer->height()),
            AV_PIX_FMT_BGRA,
            _vframe->width,
            _vframe->height,
            AV_PIX_FMT_YUV420P,
            SWS_BICUBIC,
            nullptr,
            nullptr,
            nullptr);
        if (!_nextSws) {
            THROW_ERROR("Failed to create SWS context.");
        }
    }

    _backBuffer->copyFrom(frame);
    _backBufferDirty = true;
    ++_receivedVideoFrames;
}

void FfmpegAvEncoderImpl::addAudioFrame(const service::recorder::audio::FAudioPacketView& view, size_t encoderIdx, const AVSyncClock::time_point& tm) {
    if (!_running) {
        return;
    }
    auto& streamData = *_astreams[encoderIdx];
    streamData.receivedSamples += view.props().numSamples;

    // Immediately process input audio packets to try to best achieve real-time processing.
    streamData.reinitSampleStorage(static_cast<int>(view.props().numSamples));

    const auto* inputByteBuffer = reinterpret_cast<const uint8_t*>(view.buffer());                    
    const auto numConverted = swr_convert(
        streamData.swr,
        streamData.samplesStorage,
        static_cast<int>(streamData.maxSamples),
        &inputByteBuffer,
        static_cast<int>(view.props().numSamples));

    streamData.processedSamples += view.props().numSamples;
    streamData.convertedSamples += numConverted;
    streamData.writeStoredSamplesToFifo(numConverted);

    const auto frameSize = _acodecContext->frame_size;
    while (hasAudioFrameAvailable()) {
        for (size_t s = 0; s < _astreams.size(); ++s) {
            const auto numChannels = _acodecContext->channels;

            if (av_audio_fifo_read(_astreams[s]->fifo, (void**)_aTmpFrame->data, frameSize) < frameSize) {
                THROW_ERROR("Failed to read from FIFO queue.");
            }
            _astreams[s]->encodedSamples += frameSize;

            // Copy from the tmp frame into the encoding frame.
            // We are very much assuming a planar output right now.
            for (auto c = 0; c < _acodecContext->channels; ++c) {
                const float* input = (const float*)_aTmpFrame->data[c];
                float* output = (float*)_aframe->data[c];

                for (auto i = 0; i < frameSize; ++i) {
                    if (s == 0) {
                        output[i] = input[i];
                    } else {
                        output[i] += input[i];
                    }
                }
            }
        }

        if (av_frame_make_writable(_aframe) < 0) {
            THROW_ERROR("Failed to make frame writable.");
        }

        _aframe->pts = _aFrameNum;
        _aFrameNum += frameSize;

        {
            std::unique_lock<std::mutex> guard(_encodeMutex);
            encode(_acodecContext, _avcontext, _aframe, _astream, _ptsOffset, _dtsOffset);
        }
    }
}

void FfmpegAvEncoderImpl::swapBuffers() {
    std::unique_lock<std::mutex> guard(_backMutex);
    if (!_backBufferDirty) {
        return;
    }
    _frontBuffer.swap(_backBuffer);
    _backBufferDirty = false;
    std::swap(_sws, _nextSws);
}

void FfmpegAvEncoderImpl::start() {
    if (!_vstream) {
        return;
    }

    // This is what actually gets us to write to a file.
    if (avio_open2(&_avcontext->pb, _streamUrl.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr) < 0) {
        THROW_ERROR("Failed to open video for output");
    }

    AVDictionary* mp4Options = nullptr;
    // Need empty_moov+frag_keyframe to get a fragmented mp4. We need this so that we can write to a non-seekable stream (i.e. directly to GCS).
    // We need +default_base_moof so that when Chrome tries to download the metadata for this file, it doesn't effectively try to download the whole file.
    av_dict_set(&mp4Options, "movflags", "+empty_moov+frag_keyframe+default_base_moof", 0);
    if (avformat_write_header(_avcontext, &mp4Options) < 0) {
        THROW_ERROR("Failed to write header");
    }
    av_dict_free(&mp4Options);

    av_dump_format(_avcontext, 0, _streamUrl.c_str(), 1);

    // Use this time to sync the audio and video. This time is when our video and audio should start.
    const auto start = AVSyncClock::now();

    // Start a thread to send frames and packets to the underlying encoder.
    // Some encoders (e.g. H264_NVENC) don't play nicely with variable framerate
    // so we need to force the variable framerate input that we'll get to be
    // a constant framerate. To do this we use "double buffering" where the
    // video encoder thread writes to the encoder using the front buffer while
    // the application writes to the video encoder using the back buffer. The
    // video encoder thread will run every 1/fps seconds. When the thread ticks
    // the encoder thread will swap the front and back buffer and send the new
    // front buffer to the ffmpeg encoder to write out to the file.    
    _running = true;
    _videoEncodingThread = std::thread([this, start](){
        while (_running) {
            std::unique_lock<std::mutex> pausedLock(_pauseMutex);
            _pauseCv.wait(pausedLock, [this](){ return !_pauseInput; });
            
            const auto refFrameTime = start + std::chrono::nanoseconds(_nsPerFrame * _vFrameNum);
            const auto now = AVSyncClock::now();

            // If "now" is before the refFrameTime then we want to wait until
            // refFrameTime and write a single frame. If we're running behind schedule
            // then we'll have to write multiple frames at a single time to try and
            // make up the difference.
            auto desiredFrameNum = _vFrameNum;
            if (now < refFrameTime) {
                desiredFrameNum++;
                std::this_thread::sleep_until(refFrameTime);
            } else {
                const auto elapsedFrames = std::chrono::duration_cast<std::chrono::nanoseconds>(now - refFrameTime).count() / _nsPerFrame.count();
                desiredFrameNum += std::max(elapsedFrames, long long(1));
            }

#if LOG_FRAME_TIME
            const auto taskNow = std::chrono::high_resolution_clock::now();
#endif                
            swapBuffers();

            if (_frontBuffer->isInit()) {
                const auto* inputBuffer = _frontBuffer->buffer();
                const int inputBufferStride[1] = { static_cast<int>(_frontBuffer->width() * _frontBuffer->bytesPerPixel()) };
                sws_scale(_sws, &inputBuffer, inputBufferStride, 0, static_cast<int>(_frontBuffer->height()), _vframe->data, _vframe->linesize);
                ++_processedVideoFrames;
            }

            for (; _vFrameNum < desiredFrameNum; ++_vFrameNum) {
                _vframe->pts = _vFrameNum;

                if (av_frame_make_writable(_vframe) < 0) {
                    THROW_ERROR("Failed to make video frame writeable.");
                }
                
                {
                    std::unique_lock<std::mutex> guard(_encodeMutex);
                    encode(_vcodecContext, _avcontext, _vframe, _vstream, _ptsOffset, _dtsOffset);
                }
            }
        
#if LOG_FRAME_TIME
            const auto elapsedTime = std::chrono::high_resolution_clock::now() - taskNow;
            const auto numMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();
            LOG_INFO("FFMPEG Encode: " << numMs << "ms [SKIP:" << skipFrame << "]" << std::endl);
#endif
        }
    });
}

bool FfmpegAvEncoderImpl::hasAudioFrameAvailable() const {
    for (const auto& stream : _astreams) {
        if (av_audio_fifo_size(stream->fifo) < _acodecContext->frame_size) {
            return false;
        }
    }
    return true;
}

void FfmpegAvEncoderImpl::stop() {
    if (!_running) {
        return;
    }

    _running = false;
    _videoEncodingThread.join();

    LOG_INFO("SquadOV FFMpeg Encoder Stats: " << std::endl
        << "\tVideo Frames: [Receive: " << _receivedVideoFrames  << "] [Process: " << _processedVideoFrames << "]" << std::endl
    );

    for (size_t s = 0; s < _astreams.size(); ++s) {
        LOG_INFO("\tAudio Samples [" << s 
            << "]: Receive - " << _astreams[s]->receivedSamples
            << ":: Process - " << _astreams[s]->processedSamples
            << ":: Converted - " << _astreams[s]->convertedSamples
            << ":: Encoded - " << _astreams[s]->encodedSamples
        << std::endl);
    }

    // Flush packets from encoder.
    encode(_vcodecContext, _avcontext, nullptr, _vstream, _ptsOffset, _dtsOffset);

    if (!!_acodecContext) {
        encode(_acodecContext, _avcontext, nullptr, _astream, _ptsOffset, _dtsOffset);
    }
    av_write_trailer(_avcontext);
}

void FfmpegAvEncoderImpl::pauseInputRecorderProcessing(bool pause) {
    // In the ideal case we'd queue up an image buffer so that we don't lose video frames while
    // processing the append; however the sad reality is that that just isn't feasible due to the
    // memory usage such a process would incur. A raw 1080p image is about 8MB in memory usage.
    // 1 second of 60fps footage is thus 480MB in memory usage. We can't particularly guarantee
    // that this process will complete in under a second so we'll just use a still frame to buffer.
    // Better than nothing?
    std::lock_guard<std::mutex> guard(_pauseMutex);
    _pauseInput = pause;
    if (!pause) {
        _pauseCv.notify_all();
    }
}

void FfmpegAvEncoderImpl::appendFromVideoFile(const std::filesystem::path& vodPath, int64_t startMs) {
    std::ostringstream prefix;
    prefix << "Appending from video file: " << vodPath << " -- @ " << startMs << "ms" << std::endl;
    shared::Timer timer(prefix.str());

    const std::string strPath = shared::filesystem::pathUtf8(vodPath);

    // Read in the VOD and effectively do a remux into our current file since we can assume
    // that all the codecs are equivalent.
    AVFormatContext* inputContext = nullptr;
    if (avformat_open_input(&inputContext, strPath.c_str(), nullptr, nullptr) < 0) {
        LOG_ERROR("Failed to FFMpeg open VOD path: " << vodPath << std::endl);
        return;
    }

    if (avformat_find_stream_info(inputContext, nullptr) < 0) {
        LOG_ERROR("Failed to FFMpeg find stream info: " << vodPath << std::endl);
        return;
    }

    av_dump_format(inputContext, 0, strPath.c_str(), 0);

    AVPacket inputPacket;
    int64_t vodPts = 0;
    int64_t vodDts = 0;
    while (av_read_frame(inputContext, &inputPacket) >= 0) {
        const auto inputStreamIdx = inputPacket.stream_index;
        const auto streamType = inputContext->streams[inputStreamIdx]->codecpar->codec_type;

        if (streamType == AVMEDIA_TYPE_VIDEO || streamType == AVMEDIA_TYPE_AUDIO) {
            // Note that we make the assumption that the stream indices are the same because we're using the
            // same encoder here...if this ever changes in the future then this assumption must change in the
            // logic below. Keeping this NOP here to make this logic explicit.
            AVStream* istream = inputContext->streams[inputStreamIdx];
            AVStream* ostream = _avcontext->streams[inputStreamIdx];
            
            av_packet_rescale_ts(&inputPacket, istream->time_base, ostream->time_base);

            vodPts = std::max(vodPts, inputPacket.pts);
            vodDts = std::max(vodDts, inputPacket.dts);

            // Note that we're doing a remux here not a transcode! Hence we can save ourselves from
            // doing a decode/encode. Instead we can just move the packet to the right place and call it a day! (:
            inputPacket.pts += _ptsOffset;
            inputPacket.dts += _dtsOffset;

            if (av_interleaved_write_frame(_avcontext, &inputPacket) < 0) {
                LOG_ERROR("Failed to FFMpeg write appended VOD frame." << std::endl);
                break;
            }
        }

        av_packet_unref(&inputPacket);
    }

    _dtsOffset += vodDts;
    _ptsOffset += vodPts;

    avformat_close_input(&inputContext);
}

shared::squadov::VodMetadata FfmpegAvEncoderImpl::getMetadata() const {
    shared::squadov::VodMetadata metadata;
    metadata.id = "source";

    // Bandwidth
    {
        const auto* props = reinterpret_cast<const AVCPBProperties*>(av_stream_get_side_data(
            _vstream,
            AV_PKT_DATA_CPB_PROPERTIES,
            nullptr
        ));

        if (props) {
            metadata.avgBitrate = props->avg_bitrate;
            metadata.maxBitrate = props->max_bitrate;
            metadata.minBitrate = props->min_bitrate;
        } else if (_vstream->codecpar->bit_rate) {
            metadata.avgBitrate = _vstream->codecpar->bit_rate;
            metadata.maxBitrate = _vstream->codecpar->bit_rate;
            metadata.minBitrate = _vstream->codecpar->bit_rate;
        }
    }

    // Resolution
    {
        metadata.resX = _vcodecContext->width;
        metadata.resY = _vcodecContext->height;
    }

    // FPS
    metadata.fps = static_cast<int>(static_cast<double>(_vcodecContext->framerate.num) / _vcodecContext->framerate.den);
    return metadata;
}

FfmpegAvEncoder::FfmpegAvEncoder(const std::string& streamUrl):
    _impl(new FfmpegAvEncoderImpl(streamUrl)) {  
}

FfmpegAvEncoder::~FfmpegAvEncoder() = default;

void FfmpegAvEncoder::addVideoFrame(const service::recorder::image::Image& frame) {
    _impl->addVideoFrame(frame);
}

void FfmpegAvEncoder::initializeVideoStream(size_t fps, size_t width, size_t height) {
    _impl->initializeVideoStream(fps, width, height);
}

void FfmpegAvEncoder::start() {
    _impl->start();
}

void FfmpegAvEncoder::stop() {
    _impl->stop();
}

shared::squadov::VodMetadata FfmpegAvEncoder::getMetadata() const {
    return _impl->getMetadata();
}

void FfmpegAvEncoder::getVideoDimensions(size_t& width, size_t& height) const {
    _impl->getVideoDimensions(width, height);
}

const std::string& FfmpegAvEncoder::streamUrl() const {
    return _impl->streamUrl();
}

void FfmpegAvEncoder::initializeAudioStream() {
    _impl->initializeAudioStream();
}

size_t FfmpegAvEncoder::addAudioInput(const service::recorder::audio::AudioPacketProperties& inputProps) {
    return _impl->addAudioInput(inputProps);
}

void FfmpegAvEncoder::addAudioFrame(const service::recorder::audio::FAudioPacketView& view, size_t encoderIdx, const AVSyncClock::time_point& tm) {
    _impl->addAudioFrame(view, encoderIdx, tm);
}

void FfmpegAvEncoder::pauseInputRecorderProcessing(bool pause) {
    _impl->pauseInputRecorderProcessing(pause);
}

void FfmpegAvEncoder::appendFromVideoFile(const std::filesystem::path& vodPath, int64_t startMs) {
    _impl->appendFromVideoFile(vodPath, startMs);
}

}