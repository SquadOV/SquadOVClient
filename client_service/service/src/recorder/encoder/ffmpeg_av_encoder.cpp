#include "recorder/encoder/ffmpeg_av_encoder.h"

#include "recorder/audio/fixed_size_audio_packet.h"
#include "shared/env.h"
#include "shared/errors/error.h"
#include "shared/log/log.h"
#include "shared/timer.h"
#include "shared/filesystem/utility.h"
#include "recorder/image/d3d_image.h"
#include "renderer/d3d11_renderer.h"
#include "recorder/encoder/ffmpeg_video_swap_chain.h"

#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>
#include <cstdio>
#include <chrono>
#include <cstring>
#include <deque>
#include <iostream>
#include <thread>
#include <memory>
#include <mutex>
#include <shared_mutex>
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
#include <libswresample/swresample.h>
#include <libavutil/hwcontext_d3d11va.h>
#include "recorder/encoder/ffmpeg_utils.h"
}

#define LOG_FRAME_TIME 0

using namespace std::chrono_literals;

namespace service::recorder::encoder {
namespace {

int64_t packetPropsToFFmpegChannelLayout(const service::recorder::audio::AudioPacketProperties& props) {
    if (props.numChannels == 8) {
        return AV_CH_LAYOUT_7POINT1;
    } else if (props.numChannels == 7) {
        return AV_CH_LAYOUT_6POINT1;
    } else if (props.numChannels == 6) {
        return AV_CH_LAYOUT_5POINT1;
    } else if (props.numChannels == 5) {
        return AV_CH_LAYOUT_5POINT0;
    } else if (props.numChannels == 4) {
        return AV_CH_LAYOUT_QUAD;
    } else if (props.numChannels == 3) {
        return AV_CH_LAYOUT_2POINT1;
    } else if (props.numChannels == 2) {
        return AV_CH_LAYOUT_STEREO;
    } else {
        return AV_CH_LAYOUT_MONO;
    }
}

AVSampleFormat packetPropsToFFmpegSampleFmt(const service::recorder::audio::AudioPacketProperties& props) {
    return props.isPlanar ? AV_SAMPLE_FMT_FLTP : AV_SAMPLE_FMT_FLT;
}

int packetPropsToFFmpegSampleRate(const service::recorder::audio::AudioPacketProperties& props) {
    return static_cast<int>(props.samplingRate);
}

constexpr size_t audioNumChannels = 2;
constexpr size_t audioSamplesPerFrame = 480;

}

class FfmpegAvEncoderImpl {
public:
    explicit FfmpegAvEncoderImpl(const std::string& streamUrl);
    ~FfmpegAvEncoderImpl();

    const std::string& streamUrl() const { return _streamUrl; }
    void initializeVideoStream(size_t fps, size_t width, size_t height, bool useHw);
    VideoStreamContext getVideoStreamContext() const { return _videoStreamContext; }
    void addVideoFrame(const service::recorder::image::Image& frame);
#ifdef _WIN32
    void addVideoFrame(ID3D11Texture2D* image);
#endif

    void getVideoDimensions(size_t& width, size_t& height);
    service::recorder::image::Image getFrontBuffer() const;

    void initializeAudioStream();
    size_t addAudioInput(const service::recorder::audio::AudioPacketProperties& inputProps);
    void addAudioFrame(const service::recorder::audio::FAudioPacketView& view, size_t encoderIdx, const AVSyncClock::time_point& tm);

    void open();
    void start();
    void stop();
    shared::squadov::VodMetadata getMetadata() const;
private:
    void encode(AVCodecContext* cctx, AVFrame* frame, AVStream* st);

    AVFrame* createAudioFrame() const;
    bool hasAudioFrameAvailable() const;

    std::string _streamUrl;

    // AV Output
    VideoStreamContext _videoStreamContext = VideoStreamContext::CPU;
    AVOutputFormat* _avformat = nullptr;
    AVFormatContext* _avcontext = nullptr;
    std::mutex _encodeMutex;
    std::condition_variable _encodeCv;
    std::deque<AVPacket> _packetQueue;
    std::thread _packetThread;
    void flushPacketQueue();
    
    // Video Output
    AVStream* _vstream = nullptr;
    const AVCodec* _vcodec = nullptr;
    AVCodecContext* _vcodecContext = nullptr;
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
        std::shared_mutex fifoMutex;

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

    // Front and back buffering to support constant frame rate videos.
    std::thread _videoEncodingThread;
    bool _running = false;
    std::mutex _runningMutex;
    std::chrono::nanoseconds _nsPerFrame;

    FfmpegVideoSwapChainPtr _videoSwapChain;

    uint64_t _receivedVideoFrames = 0;
    uint64_t _processedVideoFrames = 0;

    // We need this so that in the off-chance that start/stop are called around the same time
    // the stop will take precendence and not allow the start method to finish. The scenario I'm
    // particularly worried about is that we call start/stop around the same time and then the
    // order of operations becomes
    // 1) stop: Set running to false
    // 2) start: Set running to true
    // 3) start: create threads
    // 4) stop: block on thread join (won't finish until running is false)
    mutable std::mutex _startMutex;
    bool _canStart = true;
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
    std::unique_lock<std::shared_mutex> guard(fifoMutex);

    const auto newNumSamples = av_audio_fifo_space(fifo) + numSamples;
    if (av_audio_fifo_realloc(fifo, newNumSamples) < 0) {
        THROW_ERROR("Failed to reallocate FIFO queue.");
    }

    if (av_audio_fifo_write(fifo, (void**)samplesStorage, numSamples) < numSamples) {
        THROW_ERROR("Failed to write to FIFO queue.");
    }
}

FfmpegAvEncoderImpl::FfmpegAvEncoderImpl(const std::string& streamUrl):
    _streamUrl(streamUrl) {

    _avformat = av_guess_format("mpegts", nullptr, nullptr);
    if (!_avformat) {
        THROW_ERROR("Failed to find the MPEG-TS format.");
    }

    if (avformat_alloc_output_context2(&_avcontext, _avformat, nullptr, nullptr) < 0) {
        THROW_ERROR("Failed to allocate AV context.");
    }
}

FfmpegAvEncoderImpl::~FfmpegAvEncoderImpl() {
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

void FfmpegAvEncoderImpl::encode(AVCodecContext* cctx, AVFrame* frame, AVStream* st) {
    // Encode video frame.
    auto ret = avcodec_send_frame(cctx, frame);
    if (ret < 0) {
        THROW_ERROR("Failed to send frame: " << ret);
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
        packet.stream_index = st->index;

        {
            std::lock_guard<std::mutex> guard(_encodeMutex);
            _packetQueue.push_back(packet);
        }
    }
    _encodeCv.notify_all();
}

void FfmpegAvEncoderImpl::getVideoDimensions(size_t& width, size_t& height) {
    width = _videoSwapChain->frameWidth();
    height = _videoSwapChain->frameHeight();
}

void FfmpegAvEncoderImpl::initializeVideoStream(size_t fps, size_t width, size_t height, bool useHw) {
    // Try to use hardware encoding first. If not fall back on mpeg4.
    struct EncoderChoice {
        std::string name;
        VideoStreamContext ctx;
    };

    const EncoderChoice encodersToUse[] = {
        {"h264_amf", VideoStreamContext::GPU },
        {"h264_nvenc", VideoStreamContext::GPU },
        {"libopenh264", VideoStreamContext::CPU }
    };

    bool foundEncoder = false;
    bool canUseHwAccel = false;
    for (const auto& enc : encodersToUse) {
        try {
            _vcodec = avcodec_find_encoder_by_name(enc.name.c_str());
            if (!_vcodec) {
                THROW_ERROR("Failed to find the video codec.");
            }
            
            _vcodecContext = avcodec_alloc_context3(_vcodec);
            if (!_vcodecContext) {
                THROW_ERROR("Failed to allocate video context.");
            }

            _vcodecContext->width = static_cast<int>(width);
            _vcodecContext->height = static_cast<int>(height);

            canUseHwAccel = (enc.ctx == VideoStreamContext::GPU) && FfmpegGPUVideoSwapChain::isSupported(service::renderer::getSharedD3d11Context(), width, height) && useHw;
            _vcodecContext->pix_fmt = canUseHwAccel ? AV_PIX_FMT_D3D11 : AV_PIX_FMT_YUV420P;
            _vcodecContext->bit_rate = 6000000;
            _vcodecContext->thread_count = 0;
            
            if (canUseHwAccel) {
                AVBufferRef* hwContextRef = av_hwdevice_ctx_alloc(AV_HWDEVICE_TYPE_D3D11VA);
                if (!hwContextRef) {
                    THROW_ERROR("Failed to create HW context.");
                }

                AVHWDeviceContext* hwContext = reinterpret_cast<AVHWDeviceContext*>(hwContextRef->data);
                AVD3D11VADeviceContext* d3dContext = reinterpret_cast<AVD3D11VADeviceContext*>(hwContext->hwctx);

                service::renderer::D3d11SharedContext d3d;
                auto immediate = d3d.immediateContext();
                d3dContext->device = d3d.device();
                d3dContext->device_context = immediate.context();

                // This is needed to pass the ownership to ffmpeg.
                d3d.device()->AddRef();
                immediate.context()->AddRef();

                if (av_hwdevice_ctx_init(hwContextRef) < 0) {
                    av_buffer_unref(&hwContextRef);
                    THROW_ERROR("Failed to init hw context.");
                }

                AVBufferRef* frameContextRef = av_hwframe_ctx_alloc(hwContextRef);
                if (!frameContextRef) {
                    av_buffer_unref(&hwContextRef);
                    THROW_ERROR("Failed to create Frame context.");
                }

                AVHWFramesContext* frameContext = reinterpret_cast<AVHWFramesContext*>(frameContextRef->data);
                AVD3D11VAFramesContext* d3dFrameContext = reinterpret_cast<AVD3D11VAFramesContext*>(frameContext->hwctx);

                frameContext->format = _vcodecContext->pix_fmt;
                frameContext->sw_format = AV_PIX_FMT_NV12;
                frameContext->width = _vcodecContext->width;
                frameContext->height = _vcodecContext->height;
                frameContext->initial_pool_size = 1;

                d3dFrameContext->BindFlags = D3D11_BIND_RENDER_TARGET;
                d3dFrameContext->MiscFlags = D3D11_RESOURCE_MISC_SHARED;

                if (av_hwframe_ctx_init(frameContextRef) < 0) {
                    av_buffer_unref(&hwContextRef);
                    av_buffer_unref(&frameContextRef);
                    THROW_ERROR("Failed to init Frame context.");
                }

                _vcodecContext->hw_device_ctx = hwContextRef;
                _vcodecContext->hw_frames_ctx = frameContextRef;
            }

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
            _videoStreamContext = enc.ctx;
            LOG_INFO("FFmpeg Found Encoder: " << enc.name << std::endl);
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

    if (canUseHwAccel) {
        LOG_INFO("Using FFmpeg GPU Video Swap Chain" << std::endl);
        _videoSwapChain.reset(new FfmpegGPUVideoSwapChain);
    } else {
        LOG_INFO("Using FFmpeg CPU Video Swap Chain" << std::endl);
        _videoSwapChain.reset(new FfmpegCPUVideoSwapChain);
    }
    _videoSwapChain->initializeGpuSupport(service::renderer::getSharedD3d11Context());
    _videoSwapChain->initialize(_vcodecContext, _vcodecContext->hw_frames_ctx);
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
    _videoSwapChain->receiveCpuFrame(frame);

    if (_running) {
        ++_receivedVideoFrames;
    }
}

void FfmpegAvEncoderImpl::addVideoFrame(ID3D11Texture2D* image) {
    _videoSwapChain->receiveGpuFrame(image);
    
    if (_running) {
        ++_receivedVideoFrames;
    }
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

    // I don't believe there's a need to do this on multiple encoder indexes...The only
    // time this *might* screw up is if encoder index 0 receives 0 audio packets. Oh well?
    // The FIFO is protected by a shared mutex because each audio input stream is fed in from a
    // separate thread.
    if (encoderIdx == 0) {
        const auto frameSize = _acodecContext->frame_size;
        while (hasAudioFrameAvailable()) {
            for (size_t s = 0; s < _astreams.size(); ++s) {
                const auto numChannels = _acodecContext->channels;

                {
                    std::shared_lock<std::shared_mutex> guard(_astreams[s]->fifoMutex);
                    if (av_audio_fifo_read(_astreams[s]->fifo, (void**)_aTmpFrame->data, frameSize) < frameSize) {
                        THROW_ERROR("Failed to read from FIFO queue.");
                    }
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
            encode(_acodecContext, _aframe, _astream);
        }
    }
}

service::recorder::image::Image FfmpegAvEncoderImpl::getFrontBuffer() const {
    return _videoSwapChain->cpuCopyFrontBuffer();
}

void FfmpegAvEncoderImpl::open() {
    // This is what actually gets us to write to a file.
    if (avio_open2(&_avcontext->pb, _streamUrl.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr) < 0) {
        THROW_ERROR("Failed to open video for output: " << _streamUrl);
    }

    if (avformat_write_header(_avcontext, nullptr) < 0) {
        THROW_ERROR("Failed to write header");
    }
    av_dump_format(_avcontext, 0, _streamUrl.c_str(), 1);
}

void FfmpegAvEncoderImpl::start() {
    std::lock_guard<std::mutex> guard(_startMutex);

    if (!_canStart) {
        LOG_INFO("Aborting start due to existing stop call." << std::endl);
    }

    if (!_vstream) {
        return;
    }

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
    {
        std::lock_guard<std::mutex> guard(_runningMutex);
        _running = true;
    }

    _videoEncodingThread = std::thread([this, start](){
        while (_running) {
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
            _videoSwapChain->swap();

            AVFrame* frame = _videoSwapChain->getFrontBufferFrame();
            if (_videoSwapChain->hasValidFrontBuffer()) {
                ++_processedVideoFrames;
            }

            for (; _vFrameNum < desiredFrameNum; ++_vFrameNum) {
                frame->pts = _vFrameNum;
                encode(_vcodecContext, frame, _vstream);
            }
        
#if LOG_FRAME_TIME
            const auto elapsedTime = std::chrono::high_resolution_clock::now() - taskNow;
            const auto numMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();
            LOG_INFO("FFMPEG Encode: " << numMs << "ms [SKIP:" << skipFrame << "]" << std::endl);
#endif
        }

        LOG_INFO("Finish encoding thread." << std::endl);
    });

    // The packet thread is responsible for actually writing the encoded packets to the file.
    _packetThread = std::thread([this](){
        while (_running) {
            std::unique_lock<std::mutex> encodeLock(_encodeMutex);
            if (!_encodeCv.wait_for(encodeLock, std::chrono::milliseconds(100), [this](){ return !_packetQueue.empty(); })) {
                continue;
            }

            flushPacketQueue();
        }

        LOG_INFO("Finish packet thread." << std::endl);
    });
}

void FfmpegAvEncoderImpl::flushPacketQueue() {
    int ret = 0;
    while (!_packetQueue.empty()) {
        AVPacket packet = _packetQueue.front();
        _packetQueue.pop_front();

        if ((ret = av_interleaved_write_frame(_avcontext, &packet)) < 0) {
            char errBuff[2048];
            av_make_error_string(errBuff, 2048, ret);
            LOG_ERROR("Failed to write packet: " << errBuff);
            continue;
        }
        av_packet_unref(&packet);
    }
}

bool FfmpegAvEncoderImpl::hasAudioFrameAvailable() const {
    for (const auto& stream : _astreams) {
        std::shared_lock<std::shared_mutex> guard(stream->fifoMutex);
        if (av_audio_fifo_size(stream->fifo) < _acodecContext->frame_size) {
            return false;
        }
    }
    return true;
}

void FfmpegAvEncoderImpl::stop() {
    LOG_INFO("Requesting FFmpeg Encoder Stop: " << _running << std::endl);
    std::lock_guard<std::mutex> guard(_startMutex);
    _canStart = false;

    if (!_running) {
        return;
    }

    {
        std::lock_guard<std::mutex> guard(_runningMutex);
        _running = false;
    }

    LOG_INFO("Waiting for video encoding/IO threads to finish [" << _running << "]..." << std::endl);
    if (_videoEncodingThread.joinable()) {
        LOG_INFO("\tJoining video encoding thread..." << std::endl);
        _videoEncodingThread.join();
    }

    if (_packetThread.joinable()) {
        LOG_INFO("\tJoining packet thread..." << std::endl);
        _packetThread.join();
    }

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
    encode(_vcodecContext, nullptr, _vstream);

    if (!!_acodecContext) {
        encode(_acodecContext, nullptr, _astream);
    }

    flushPacketQueue();
    av_write_trailer(_avcontext);
    LOG_INFO("Finish FFMpeg write: " << _streamUrl << std::endl);
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

#ifdef _WIN32
void FfmpegAvEncoder::addVideoFrame(ID3D11Texture2D* image) {
    _impl->addVideoFrame(image);
}
#endif

void FfmpegAvEncoder::initializeVideoStream(size_t fps, size_t width, size_t height, bool useHw) {
    _impl->initializeVideoStream(fps, width, height, useHw);
}

VideoStreamContext FfmpegAvEncoder::getVideoStreamContext() const {
    return _impl->getVideoStreamContext();
}

service::recorder::image::Image FfmpegAvEncoder::getFrontBuffer() const {
    return _impl->getFrontBuffer();
}

void FfmpegAvEncoder::open() {
    _impl->open();
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

}