#include "recorder/encoder/ffmpeg_av_encoder.h"

#include "recorder/audio/fixed_size_audio_packet.h"
#include "shared/env.h"
#include "shared/errors/error.h"
#include "shared/log/log.h"
#include "shared/timer.h"
#include "shared/filesystem/utility.h"
#include "shared/filesystem/common_paths.h"
#include "shared/audio/timing.h"
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
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
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
std::string ffmpegErrorString(int ret) {
    char buffer[512];
    av_make_error_string(buffer, 512, ret);
    return std::string(buffer);
}

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

std::string ffmpegChannelLayoutToAformatString(int64_t layout) {
    char buffer[512];
    av_get_channel_layout_string(buffer, 512, 0, layout);
    return std::string(buffer);
}

AVSampleFormat packetPropsToFFmpegSampleFmt(const service::recorder::audio::AudioPacketProperties& props) {
    return props.isPlanar ? AV_SAMPLE_FMT_FLTP : AV_SAMPLE_FMT_FLT;
}
int packetPropsToFFmpegSampleRate(const service::recorder::audio::AudioPacketProperties& props) {
    return static_cast<int>(props.samplingRate);
}

constexpr size_t audioNumChannels = 2;
constexpr size_t audioSamplesPerFrame = 480;
const std::chrono::nanoseconds maxAudioDeviation = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(80));

}

class FfmpegAvEncoderImpl {
public:
    explicit FfmpegAvEncoderImpl(const std::string& streamUrl);
    ~FfmpegAvEncoderImpl();

    const std::string& streamUrl() const { return _streamUrl; }
    void initializeVideoStream(const service::renderer::D3d11SharedContextPtr& d3d, const service::system::RecordingSettings& settings, size_t width, size_t height);
    VideoStreamContext getVideoStreamContext() const { return _videoStreamContext; }
    AVSyncClock::time_point getSyncStartTime() const;

#ifdef _WIN32
    void addVideoFrame(ID3D11Texture2D* image, size_t numFrames);
#endif

    void finalizeStreams();
    void resizeDvrBuffer(double timeSeconds);
    void getVideoDimensions(size_t& width, size_t& height);

    void initializeAudioStream();
    size_t addAudioInput(const service::recorder::audio::AudioPacketProperties& inputProps, const AudioInputSettings& settings);
    void addAudioFrame(const service::recorder::audio::FAudioPacketView& view, size_t encoderIdx, const AVSyncClock::time_point& tm);

    shared::TimePoint open(const std::string& outputUrl, std::optional<AVSyncClock::time_point> dvrFillInStartTime);
    void start();
    void stop();
    shared::squadov::VodMetadata getMetadata() const;
private:
    void encode(AVCodecContext* cctx, AVFrame* frame, AVStream* st);

    AVFrame* createAudioFrame() const;
    bool hasAudioFrameAvailable() const;

    std::string _streamUrl;

    std::atomic_bool _fileOutputReady = false;
    std::atomic_bool _dvrBufferReady = false;

    // AV Output
    VideoStreamContext _videoStreamContext = VideoStreamContext::CPU;
    AVOutputFormat* _avformat = nullptr;
    AVFormatContext* _avcontext = nullptr;
    std::mutex _encodeMutex;
    std::condition_variable _encodeCv;
    std::deque<AVPacket> _packetQueue;
    std::thread _packetThread;
    void flushPacketQueue();
    
    AVSyncClock::time_point _syncStartTime;
    // Video Output
    AVStream* _vstream = nullptr;
    const AVCodec* _vcodec = nullptr;
    AVCodecContext* _vcodecContext = nullptr;
    std::atomic<int64_t> _vFrameNum = 0;
    void videoEncodeFrame(AVFrame* frame, size_t numFrames);

    // Audio Output
    const AVCodec* _acodec = nullptr;
    AVCodecContext* _acodecContext = nullptr;
    AVStream* _astream = nullptr;
    AVFrame* _aframe = nullptr;
    std::atomic<int64_t> _aFrameNum = 0;

    struct AudioStreamData {
        // *Output* packet queue. Once we process the input
        // packet, we'll convert it into whatever format
        // is needed for output and store it in this queue.
        // Once the queue reaches the size of an actual frame,
        // we'll send that to the encoder to be written out.
        AVSyncClock::time_point nextPacketTime;
        std::shared_mutex graphMutex;

        // Filter Graph
        AVFilterGraph* filterGraph = nullptr;
        AVFilterContext* sourceCtx = nullptr;
        AVFilterContext* sinkCtx = nullptr;
        bool useSilenceCompensation = true;

        AVFrame* sourceFrame = nullptr;
        std::deque<AVFrame*> sinkFrames;

        void initializeSourceFrame(size_t numSamples, const service::recorder::audio::AudioPacketProperties& props);

        // Debug Info
        uint64_t receivedSamples = 0;
        uint64_t processedSamples = 0;
        uint64_t convertedSamples = 0;
        uint64_t encodedSamples = 0;

        AudioStreamData() = default;
        AudioStreamData(const AudioStreamData&) = delete;
        AudioStreamData(AudioStreamData&&) = delete;

        size_t addAudioFrame(const service::recorder::audio::FAudioPacketView& view, size_t offset);

        ~AudioStreamData();
    };
    using AudioStreamDataPtr = std::unique_ptr<AudioStreamData>;
    std::vector<AudioStreamDataPtr> _astreams;

    // Front and back buffering to support constant frame rate videos.
    bool _running = false;
    std::shared_mutex _runningMutex;
    std::chrono::nanoseconds _nsPerFrame;
    size_t _fps = 0;
    bool _useVfr4 = true;

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

    bool _doPostVideoFlush = true;
    service::renderer::D3d11SharedContextPtr _d3d;

    void addPacketToFileOutput(AVPacket& packet);
    // DVR Buffer.
    mutable std::shared_mutex _dvrMutex;
    // One per stream.
    std::vector<int64_t> _streamPtsOffset;
    std::vector<std::deque<AVPacket>> _dvrBuffer;
    std::atomic<double> _maxDvrBufferTimeSeconds = 0.0;

    bool _isHlsStream = false;
    void addPacketToDvrBuffer(const AVPacket& packet);
    bool isDvrBufferLongerThanMaxSeconds(const std::deque<AVPacket>& buffer, int streamIndex) const;
};

FfmpegAvEncoderImpl::AudioStreamData::~AudioStreamData() {
    avfilter_graph_free(&filterGraph);
    av_frame_free(&sourceFrame);
    
    for (auto& f : sinkFrames) {
        av_frame_free(&f);
    }
    sinkFrames.clear();
}

size_t FfmpegAvEncoderImpl::AudioStreamData::addAudioFrame(const service::recorder::audio::FAudioPacketView& view, size_t offset) {
    const auto numSamplesToUse = view.props().numSamples - offset;
    auto ret = 0;

    if (numSamplesToUse > 0) {
        receivedSamples += numSamplesToUse;
        
        // Copy data into a newly allocated frame.
        initializeSourceFrame(numSamplesToUse, view.props());

        if (av_frame_get_buffer(sourceFrame, 0) < 0) {
            THROW_ERROR("Failed to get audio buffer.");
        }

        if (!view.props().isPlanar) {
            memcpy(sourceFrame->data[0], view.offsetBuffer(offset), numSamplesToUse * view.props().numChannels * sizeof(float));

            float* volumeAdjustment = reinterpret_cast<float*>(sourceFrame->data[0]);
            for (auto i = 0; i < numSamplesToUse * view.props().numChannels; ++i) {
                volumeAdjustment[i] *= view.volume();
            }
        } else {
            THROW_ERROR("Planar audio input is not supported.");
        }

        // Pass this frame to the filter graph.
        std::lock_guard guard(graphMutex);
        ret = av_buffersrc_add_frame_flags(sourceCtx, sourceFrame, AV_BUFFERSRC_FLAG_KEEP_REF);
        if (ret < 0) {
            LOG_ERROR("Failed to add audio data to filter graph source [" << ffmpegErrorString(ret) << "]" << std::endl);
            return 0;
        }

        // Retrieve filtered audio from the filter graph (if any)
        // and make it available for use.
        AVFrame* tmpFrame = av_frame_alloc();
        while (true) {
            ret = av_buffersink_get_frame(sinkCtx, tmpFrame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }

            if (ret < 0) {
                LOG_ERROR("Failed to retrieve audio data from filter graph sink." << std::endl);
                return 0;
            }

            sinkFrames.push_back(av_frame_clone(tmpFrame));
            av_frame_unref(tmpFrame);
        }
        av_frame_free(&tmpFrame);
        av_frame_unref(sourceFrame);
    }
    return numSamplesToUse;
}

void FfmpegAvEncoderImpl::AudioStreamData::initializeSourceFrame(size_t numSamples, const service::recorder::audio::AudioPacketProperties& props) {
    if (sourceFrame && sourceFrame->nb_samples == numSamples) {
        return;
    }

    if (sourceFrame) {
        av_frame_free(&sourceFrame);
        sourceFrame = nullptr;
    }

    sourceFrame = av_frame_alloc();
    sourceFrame->nb_samples = numSamples;
    sourceFrame->format = packetPropsToFFmpegSampleFmt(props);
    sourceFrame->channel_layout = packetPropsToFFmpegChannelLayout(props);
    sourceFrame->channels = static_cast<int>(props.numChannels);
    sourceFrame->sample_rate = packetPropsToFFmpegSampleRate(props);
    sourceFrame->pts = 0;
}


FfmpegAvEncoderImpl::FfmpegAvEncoderImpl(const std::string& streamUrl):
    _streamUrl(streamUrl) {

    _avformat = av_guess_format(
        (streamUrl.find("rtmp://") != std::string::npos) ? "flv" :
            (streamUrl.find(".m3u8") != std::string::npos) ? "hls" : "mpegts",
        nullptr,
        nullptr
    );

    if (!_avformat) {
        THROW_ERROR("Failed to find the MPEG-TS format.");
    }

    LOG_INFO("Using AV Format: " << _avformat->name << std::endl);

    if (avformat_alloc_output_context2(&_avcontext, _avformat, nullptr, nullptr) < 0) {
        THROW_ERROR("Failed to allocate AV context.");
    }

    LOG_INFO("Using FFmpeg AV Encoder: " << streamUrl << std::endl);
}

FfmpegAvEncoderImpl::~FfmpegAvEncoderImpl() {
    auto immediate = _d3d->immediateContext();

    avcodec_free_context(&_vcodecContext);
    av_frame_free(&_aframe);
    avcodec_free_context(&_acodecContext);

    if (_avcontext->pb) {
        avio_flush(_avcontext->pb);
        avio_closep(&_avcontext->pb);
    }
    avformat_free_context(_avcontext);
}

AVSyncClock::time_point FfmpegAvEncoderImpl::getSyncStartTime() const {
    return _syncStartTime;
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

        packet.stream_index = st->index;

        if (packet.dts > packet.pts) {
            // Possible if there's a jump in the pts.
            packet.pts = packet.dts;
        }

        if (_isHlsStream) {
            packet.duration = 1;
        }

        {
            std::lock_guard<std::mutex> guard(_encodeMutex);
            _packetQueue.push_back(packet);
        }
    }
    _encodeCv.notify_all();
}

void FfmpegAvEncoderImpl::finalizeStreams() {
    if (!_avcontext) {
        return;
    }
    const auto numStreams = _avcontext->nb_streams;
    LOG_INFO("Number of Streams: " << numStreams << std::endl);
    _streamPtsOffset.resize(numStreams);
    _dvrBuffer.resize(numStreams);
}

void FfmpegAvEncoderImpl::resizeDvrBuffer(double timeSeconds) {
    _maxDvrBufferTimeSeconds = timeSeconds;
    _dvrBufferReady = true;
}

void FfmpegAvEncoderImpl::getVideoDimensions(size_t& width, size_t& height) {
    width = _videoSwapChain->frameWidth();
    height = _videoSwapChain->frameHeight();
}

void FfmpegAvEncoderImpl::initializeVideoStream(const service::renderer::D3d11SharedContextPtr& d3d, const service::system::RecordingSettings& settings, size_t width, size_t height) {
    // Try to use hardware encoding first. If not fall back on mpeg4.
    struct EncoderChoice {
        std::string name;
        VideoStreamContext ctx;
        bool isGpuEncoder;
    };

    const EncoderChoice encodersToUse[] = {
        {"h264_nvenc", VideoStreamContext::GPU, true },
        {"h264_amf", VideoStreamContext::GPU, true },
        //{"h264_mf", VideoStreamContext::CPU, false },
        {"libopenh264", VideoStreamContext::CPU, false }
    };

    LOG_INFO("Check can use GPU..." << std::endl);
    const auto canUseGpu = (d3d->deviceClass() == service::renderer::D3d11Device::GPU) && FfmpegGPUVideoSwapChain::isSupported(d3d.get(), width, height);
    LOG_INFO("..." << canUseGpu << std::endl);
    auto immediate = d3d->immediateContext();

    bool foundEncoder = false;
    bool canUseHwAccel = false;
    for (const auto& enc : encodersToUse) {
        LOG_INFO("Checking Encoder: " << enc.name << std::endl);
        try {
            if (enc.isGpuEncoder && !settings.useHwEncoder) {
                LOG_INFO("Skipping " << enc.name << " since user has disabled GPU encoding." << std::endl);
                continue;
            }

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

            canUseHwAccel = (enc.ctx == VideoStreamContext::GPU) && canUseGpu && settings.useVideoHw2;
            _vcodecContext->pix_fmt = canUseHwAccel ? AV_PIX_FMT_D3D11 : AV_PIX_FMT_YUV420P;
            _vcodecContext->bit_rate = static_cast<int64_t>(settings.bitrateKbps) * 1000;

            if (settings.useCbr) {
                _vcodecContext->rc_min_rate = _vcodecContext->bit_rate;
                _vcodecContext->rc_max_rate = _vcodecContext->bit_rate;
                _vcodecContext->rc_buffer_size = _vcodecContext->bit_rate;
            } else {
                _vcodecContext->rc_min_rate = _vcodecContext->bit_rate * 0.5;
                _vcodecContext->rc_max_rate = _vcodecContext->bit_rate * 1.5;
                _vcodecContext->rc_buffer_size = _vcodecContext->rc_max_rate * 4;
            }

            _vcodecContext->thread_count = 0;

            if (enc.name == "h264_amf") {
                // Not sure why but this having this option > 0 causes issues using the AMD AMF encoder.
                _vcodecContext->max_b_frames = 0;
                _doPostVideoFlush = false;
            } else {
                _vcodecContext->max_b_frames = 2;
            }
            
            if (canUseHwAccel) {
                AVBufferRef* hwContextRef = av_hwdevice_ctx_alloc(AV_HWDEVICE_TYPE_D3D11VA);
                if (!hwContextRef) {
                    THROW_ERROR("Failed to create HW context.");
                }

                AVHWDeviceContext* hwContext = reinterpret_cast<AVHWDeviceContext*>(hwContextRef->data);
                AVD3D11VADeviceContext* d3dContext = reinterpret_cast<AVD3D11VADeviceContext*>(hwContext->hwctx);
                
                d3dContext->device = d3d->device();
                d3dContext->device_context = immediate.context();

                // This is needed to share the ownership to ffmpeg.
                d3dContext->device->AddRef();
                d3dContext->device_context->AddRef();

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
                _vcodecContext->colorspace = AVCOL_SPC_BT709;
            }

            // We're going to specify time in ms (instead of frames) so set the timebase to 1ms.
            _vcodecContext->time_base = AVRational{1, static_cast<int>(settings.fps)};
            _vcodecContext->framerate = AVRational{static_cast<int>(settings.fps), 1};

            // A smaller gop size results in better quality (most likely) and makes it easier for vides to be cut fast for server side clipping more accurately.
            _vcodecContext->gop_size = static_cast<int>(settings.fps * 1);
           
            // Actually initialize the codec context.
            AVDictionary *options = nullptr;

            if (_vcodecContext->codec_id == AV_CODEC_ID_H264) {
                // TODO: Change this to a higher quality preset. HOWEVER
                // before we do that we'll need a better way of computing the pts
                // for each video/audio frame that comes in.
                av_dict_set(&options, "preset", "medium", 0);

                // H264 Profile
                if (enc.name == "h264_mf") {
                    _vcodecContext->profile = FF_PROFILE_H264_HIGH;
                } else {
                    av_dict_set(&options, "profile", "high", 0);
                }

                if (enc.name == "h264_amf") {
                    av_dict_set_int(&options, "header_spacing", _vcodecContext->gop_size, 0);
                }

                // CBR vs VBR
                if (!settings.useCbr) {
                    LOG_INFO("...Using VBR." << std::endl);
                    // Per-encoder settings. Mainly for VBR.
                    if (enc.name == "h264_mf") {
                        // FFmpeg uses the bitrate as the mean bit rate to use.
                        av_dict_set(&options, "rate_control", "pc_vbr", 0);
                    } else if (enc.name == "h264_nvenc") {
                        // bit_rate => averageBitRate
                        // rc_max_rate => maxBitRate
                        // rc_buffer_size => vbvBufferSize
                        av_dict_set(&options, "rc", "vbr", 0);
                    } else if (enc.name == "h264_amf") {
                        av_dict_set(&options, "rc", "vbr_peak", 0);
                    } else if (enc.name == "libopenh264") {
                        av_dict_set(&options, "rc_mode", "bitrate", 0);
                    }
                } else {
                    LOG_INFO("...Using CBR." << std::endl);
                    if (enc.name == "h264_mf") {
                        // FFmpeg uses the bitrate as the mean bit rate to use.
                        av_dict_set(&options, "rate_control", "cbr", 0);
                    } else if (enc.name == "h264_nvenc") {
                        av_dict_set(&options, "rc", "cbr", 0);
                    } else if (enc.name == "h264_amf") {
                        av_dict_set(&options, "rc", "cbr", 0);
                    } else if (enc.name == "libopenh264") {
                        av_dict_set(&options, "rc_mode", "timestamp", 0);
                    }
                }
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
    _videoSwapChain->initializeGpuSupport(d3d.get());
    _videoSwapChain->initialize(_vcodecContext, _vcodecContext->hw_frames_ctx);
    _fps = settings.fps;
    _useVfr4 = settings.useVfr4;
    _nsPerFrame = std::chrono::nanoseconds(static_cast<size_t>(1.0 / settings.fps * 1.0e+9));
    _d3d = d3d;
    assert(_d3d);
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

size_t FfmpegAvEncoderImpl::addAudioInput(const service::recorder::audio::AudioPacketProperties& inputProps, const AudioInputSettings& settings) {
    AudioStreamDataPtr streamDataPtr = std::make_unique<AudioStreamData>();
    AudioStreamData& streamData = *streamDataPtr;

    // Setup audio filter graph. We need to setup a buffer and buffer sink programatically
    // to denote the start and end of the graph.
    streamData.filterGraph = avfilter_graph_alloc();

    if (!streamData.filterGraph) {
        THROW_ERROR("Failed to create audio filter graph.");
    }

    // Need to manually create two filters and set what the expected input/output data formats are.
    {
        std::ostringstream bufferDesc;
        bufferDesc << "sample_rate=" << packetPropsToFFmpegSampleRate(inputProps)
            << ":sample_fmt=" << av_get_sample_fmt_name(packetPropsToFFmpegSampleFmt(inputProps))
            << ":channel_layout=" << ffmpegChannelLayoutToAformatString(packetPropsToFFmpegChannelLayout(inputProps));

        const AVFilter* abuffersrc = avfilter_get_by_name("abuffer");
        if (avfilter_graph_create_filter(&streamData.sourceCtx, abuffersrc, "in", bufferDesc.str().c_str(), nullptr, streamData.filterGraph) < 0) {
            THROW_ERROR("Failed to create audio filter graph buffer source.");
        }
    }

    {
        const AVFilter* abuffersink = avfilter_get_by_name("abuffersink");
        if (avfilter_graph_create_filter(&streamData.sinkCtx, abuffersink, "out", nullptr, nullptr, streamData.filterGraph) < 0) {
            THROW_ERROR("Failed to create audio filter graph buffer sink.");
        }

        const AVSampleFormat outSampleFormats[] = { _acodecContext->sample_fmt, static_cast<AVSampleFormat>(-1) };
        const int64_t outChannelLayouts[] = { static_cast<int64_t>(_acodecContext->channel_layout), -1 };
        const int outSampleRates[] = { _acodecContext->sample_rate, -1 };

        if (av_opt_set_int_list(streamData.sinkCtx, "sample_fmts", outSampleFormats, -1, AV_OPT_SEARCH_CHILDREN) < 0) {
            THROW_ERROR("Failed to set sink sample format.");
        }

        if (av_opt_set_int_list(streamData.sinkCtx, "channel_layouts", outChannelLayouts, -1, AV_OPT_SEARCH_CHILDREN) < 0) {
            THROW_ERROR("Failed to set sink channel layout.");
        }

        if (av_opt_set_int_list(streamData.sinkCtx, "sample_rates", outSampleRates, -1, AV_OPT_SEARCH_CHILDREN) < 0) {
            THROW_ERROR("Failed to set sink sample rate.");
        }
    }

    // Create the endpoints on the graph - ffmpeg will be able to connect to the
    // endpoints from the parsed graph description automatically.
    AVFilterInOut* outputs = avfilter_inout_alloc();
    AVFilterInOut* inputs  = avfilter_inout_alloc();

    if (!inputs || !outputs) {
        THROW_ERROR("Failed to create filter graph inputs/outputs.");
    }

    outputs->name = av_strdup("in");
    outputs->filter_ctx = streamData.sourceCtx;
    outputs->pad_idx = 0;
    outputs->next = nullptr;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = streamData.sinkCtx;
    inputs->pad_idx = 0;
    inputs->next = nullptr;

    // We need to construct these two filters always:
    // - aresample: Needed to get to the desired output sample rate
    // - aformat: Needed to get to the desired output channel layout and sample format
    std::ostringstream graphDescription;

    // aresample
    graphDescription << "aresample=" << _acodecContext->sample_rate;

    // aformat
    graphDescription << ",aformat=sample_fmts=" << av_get_sample_fmt_name(_acodecContext->sample_fmt) << ":channel_layouts=" << ffmpegChannelLayoutToAformatString(_acodecContext->channel_layout);
    
    // compand
    if (settings.useNoiseThreshold) {
        graphDescription << ",compand=.3:.8:-900/-900|" << settings.noiseThresholDb << ".1/-900|" << settings.noiseThresholDb << "/" << settings.noiseThresholDb << ":.01:0:-90:.1";
    }

    // arnndn
    if (settings.useSpeechNoiseReduction) {
        graphDescription << ",arnndn=m=models/sh.rnnn";
    }

    if (avfilter_graph_parse_ptr(streamData.filterGraph, graphDescription.str().c_str(), &inputs, &outputs, nullptr) < 0) {
        THROW_ERROR("Failed to parse audio graph.");
    }

    if (avfilter_graph_config(streamData.filterGraph, nullptr) < 0) {
        THROW_ERROR("Failed to validate graph.");
    }

    streamData.useSilenceCompensation = settings.useSilenceCompensation;
    av_buffersink_set_frame_size(streamData.sinkCtx, _acodecContext->frame_size);

    _astreams.emplace_back(std::move(streamDataPtr));
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    return _astreams.size() - 1;
}

void FfmpegAvEncoderImpl::addVideoFrame(ID3D11Texture2D* image, size_t numFrames) {
    std::shared_lock<std::shared_mutex> guard(_runningMutex);
    if (!_running || !numFrames) {
        return;
    }

    if (_running) {
        ++_receivedVideoFrames;
    }

    videoEncodeFrame(_videoSwapChain->receiveGpuFrame(image), numFrames);
}


void FfmpegAvEncoderImpl::videoEncodeFrame(AVFrame* frame, size_t numFramesToEncode) {
    if (!frame) {
        LOG_WARNING("Invalid frame [nullptr] - ignoring." << std::endl);
        return;
    }

    // Get this number again just in case it took us awhile to receive the frame.
    if (numFramesToEncode >= _fps) {
        LOG_WARNING("Encoding > 1 second worth of frames at a single time: " << numFramesToEncode << std::endl);
    } else if (!numFramesToEncode) {
        LOG_WARNING("Attempting to encode 0 frames...ignoring." << std::endl);
        return;
    }

    // This is 1 greater than the actual pts to send to FFmpeg.
    // I.e. when we're at frame 0 and we want to encode 1 frame,
    // start at frame 0 and not at frame 1.
    const auto desiredFrameNum = _vFrameNum + numFramesToEncode;
    ++_processedVideoFrames;

    bool forceFrame0 = false;
    
    int64_t startEncodingFrame = _vFrameNum;
    int64_t frameStepSize = 1;

    if (_useVfr4) {
        forceFrame0 = (_vFrameNum == 0 && desiredFrameNum > 1);

        // For some god awful reason, if we try to jump the pts by 600 frames or more,
        // the processing step will fail and ffmpeg will freak the fuck out and just write
        // each frame in rapid fire succession ignoring the jump once we convert to mp4.
        // Therefore, in case that happens we'll break the number of frames into smaller chunks
        // to force ffmpeg to behave. Note that we use 400 here instead of 600 just in case
        // I'm off in my estimation since it was ~600 frames on my machine but could be different
        // on machines in the wild.
        if (numFramesToEncode > 400) {
            constexpr int64_t STEP_SIZE = 200;
            startEncodingFrame = (desiredFrameNum - 1) - ((numFramesToEncode - 1) / STEP_SIZE) * STEP_SIZE;
            frameStepSize = STEP_SIZE;
        } else {
            startEncodingFrame = desiredFrameNum - 1;
        }
    }

    auto immediate = _d3d->immediateContext();

    if (forceFrame0) {
        frame->pts = 0;
        encode(_vcodecContext, frame, _vstream);
    }

    for (_vFrameNum = startEncodingFrame; _vFrameNum < desiredFrameNum; _vFrameNum += frameStepSize) {
        frame->pts = _vFrameNum;
        encode(_vcodecContext, frame, _vstream);
    }

    // This is necessary when frameStepSize > 1. At the end, we want _vFrameNum to be equal to desiredFrameNum.
    // If frameStepSize > 1, then the last frame will be desiredFrameNum - 1 + frameStepSize instead.
    _vFrameNum = desiredFrameNum;
}

void FfmpegAvEncoderImpl::addAudioFrame(const service::recorder::audio::FAudioPacketView& view, size_t encoderIdx, const AVSyncClock::time_point& tm) {
    std::shared_lock<std::shared_mutex> guard(_runningMutex);
    if (!_running) {
        return;
    }

    // The packet is coming from before we actually even started recording? Probably safe to discard in that case.
    if (tm < _syncStartTime) {
        return;
    }
    
    size_t numSilenceSamples = 0;

    auto& streamData = *_astreams[encoderIdx];

    const auto diffTm = (tm >= streamData.nextPacketTime) ? tm - streamData.nextPacketTime : streamData.nextPacketTime - tm;
    if (streamData.useSilenceCompensation && diffTm >= maxAudioDeviation) {
        if (tm > streamData.nextPacketTime) {
            // When the audio samples are coming further out into the future than we'd reasonably expect then we should
            // insert a bunch of silence in to compensate.
            numSilenceSamples = shared::audio::timeDiffToNumSamples(view.props().samplingRate, diffTm);
            LOG_WARNING("Inserting " << numSilenceSamples << " samples to compensate for audio drift of " << diffTm.count() << "ns." << std::endl);
        }
    }

    if (numSilenceSamples > 0) {
        // Create a fake packet with the appropriate number of samples containing just silence.
        std::vector<float> silenceData(numSilenceSamples * view.props().numChannels, 0.f);

        auto props = view.props();
        props.numSamples = numSilenceSamples;

        service::recorder::audio::FAudioPacketView silence(silenceData.data(), props, 0.0);
        streamData.addAudioFrame(silence, 0);
    }

    const auto totalSamplesAdded = streamData.addAudioFrame(view, 0);
    if (numSilenceSamples > 0) {
        streamData.nextPacketTime = tm;    
    }
    streamData.nextPacketTime += shared::audio::samplesToNsDiff(view.props().samplingRate, totalSamplesAdded);
    
    // I don't believe there's a need to do this on multiple encoder indexes...The only
    // time this *might* screw up is if encoder index 0 receives 0 audio packets. Oh well?
    // The FIFO is protected by a shared mutex because each audio input stream is fed in from a
    // separate thread.
    if (encoderIdx == 0) {
        const auto frameSize = _acodecContext->frame_size;
        while (hasAudioFrameAvailable()) {
            for (size_t s = 0; s < _astreams.size(); ++s) {
                const auto numChannels = _acodecContext->channels;
                _astreams[s]->encodedSamples += frameSize;

                // Copy from the sink frame into the encoding frame.
                // We are very much assuming a planar output right now.
                {
                    std::lock_guard guard(_astreams[s]->graphMutex);

                    auto* frame = _astreams[s]->sinkFrames.front();
                    _astreams[s]->sinkFrames.pop_front();

                    for (auto c = 0; c < _acodecContext->channels; ++c) {
                        const float* input = (const float*)frame->data[c];
                        float* output = (float*)_aframe->data[c];

                        for (auto i = 0; i < frameSize; ++i) {
                            if (s == 0) {
                                output[i] = input[i];
                            } else {
                                output[i] += input[i];
                            }
                        }
                    }

                    av_frame_free(&frame);
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

shared::TimePoint FfmpegAvEncoderImpl::open(const std::string& outputUrl, std::optional<AVSyncClock::time_point> dvrFillInStartTime) {
    _streamUrl = outputUrl;
    // This is what actually gets us to write to a file.
    if (!(_avcontext->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open2(&_avcontext->pb, _streamUrl.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr) < 0) {
            THROW_ERROR("Failed to open video for output: " << _streamUrl);
        }
    }

    AVDictionary* headerOptions = nullptr;

    if (strcmp(_avformat->name, "hls") == 0) {
        _avcontext->url = av_strdup(_streamUrl.c_str());

        av_dict_set(&headerOptions, "hls_time", "3.0", 0);
        av_dict_set_int(&headerOptions, "hls_list_size", 12, 0);
        av_dict_set(&headerOptions, "hls_flags", "delete_segments", 0);
        av_dict_set(&headerOptions, "hls_segment_type", "mpegts", 0);

        _isHlsStream = true;
    }

    if (avformat_write_header(_avcontext, &headerOptions) < 0) {
        THROW_ERROR("Failed to write header");
    }
    av_dict_free(&headerOptions);
    av_dump_format(_avcontext, 0, _streamUrl.c_str(), 1);

    shared::TimePoint retVodStartTime;
    if (dvrFillInStartTime && _avcontext) {
        std::shared_lock guard(_dvrMutex);

        // At this point we need to determine which packets to copy into the newly created stream.
        // We also need to determine what number we're going to subtract off of all future pts/dts for
        // each stream.
        //
        // We're going to assume pts = 0 happens at _syncStartTime. So we just need to compute the offset
        // from that moment in time to the passed in dvrFillInStartTime, convert that to each stream's time base
        // and that'll be each stream's offset.
        //
        // Hacky but works hopefully?
        //
        // Another thing to note in this block of code is that the AVStream time base is not necessarily the
        // time base of the packet. The time base of the packet should be the codec context. The AVStream time base
        // will be the timebase that we output in.
        const auto diffMs = std::chrono::duration_cast<std::chrono::milliseconds>(dvrFillInStartTime.value() - _syncStartTime).count();
        assert(_avcontext->nb_streams >= 1);

        // One tricky thing is we want to modify the actual offset a little bit so that the video starts exactly
        // when the first video stream comes in. So we first need to figure out the first packet in the video stream
        // that is past the threshold and then strip away audio packets that are earlier than the 1st video packet's PTS.
        const auto minDiffPts = av_rescale_q(diffMs, AVRational{1, 1000}, _vcodecContext->time_base);

        int64_t videoOffsetPts = minDiffPts;
        for (const auto& p: _dvrBuffer[0]) {
            if (p.pts < minDiffPts || !(p.flags & AV_PKT_FLAG_KEY)) {
                continue;
            }

            videoOffsetPts = p.pts;
            break;
        }

        // Now, we can convert the video PTS offset to the appropriate audio PTS offset.
        // Then we need to copy packets from DVR into the file output. Any packet with a PTS less than the stream offset is gucci.
        // We also want to keep track of which packet index in each buffer is the first packet we want to start copying.
        // We'll need this info afterwards in the copy step.
        std::vector<size_t> currentDvrPointers(_avcontext->nb_streams);

        _streamPtsOffset[0] = videoOffsetPts;
        LOG_INFO("Diagnostic DVR PTS [Diff PTS: " << minDiffPts << " - Diff MS: " << diffMs << "]" << std::endl);
        for (auto i = 0; i < _avcontext->nb_streams; ++i) {
            AVRational codecTimebase;
            if (i > 0) {
                _streamPtsOffset[i] = av_rescale_q(_streamPtsOffset[0], _vcodecContext->time_base, _acodecContext->time_base);
                codecTimebase = _acodecContext->time_base;
            } else {
                codecTimebase = _vcodecContext->time_base;
            }
            LOG_INFO("Stream DVR Offset [" << i << "]: " << _streamPtsOffset[i] << std::endl);
            if (_dvrBuffer[i].empty()) {
                LOG_INFO("...No DVR packets." << std::endl);
            } else {
                LOG_INFO("\tOldest DVR Packet: " << _dvrBuffer[i].front().pts << std::endl);
                LOG_INFO("\tNewest DVR Packet: " << _dvrBuffer[i].back().pts << std::endl);
                LOG_INFO("\tTime Base: " << codecTimebase.num << "/" << codecTimebase.den << std::endl);
                LOG_INFO("\tDVR Buffer Time (ms): " << av_rescale_q(_dvrBuffer[i].back().pts - _dvrBuffer[i].front().pts, codecTimebase, AVRational{1, 1000}) << std::endl);
            }
            LOG_INFO("\tDVR Buffer Count: " << _dvrBuffer[i].size() << std::endl);

            for (currentDvrPointers[i] = 0; currentDvrPointers[i] < _dvrBuffer[i].size(); ++currentDvrPointers[i]) {
                const auto& p = _dvrBuffer[i][currentDvrPointers[i]];
                const bool isKeyframe = (p.flags & AV_PKT_FLAG_KEY);

                if (p.pts < _streamPtsOffset[i] || !isKeyframe) {
                    continue;
                }

                break;
            }
        }

        // Now we need to do a thing where we insert packets in an interleaved fashion. We should not dump all the video data first followed by all
        // the audio data as that'll cause issues in processing. Instead, we should write the earliest packet at any given time until we run out of packets.
        while (true) {
            bool hasDvrPacketsLeft = false;

            int64_t smallestMs = std::numeric_limits<int64_t>::max();
            unsigned int selectedStream = _avcontext->nb_streams;
            for (auto i = 0; i < _avcontext->nb_streams; ++i) {
                const auto& buffer = _dvrBuffer[i];
                const auto ptr = currentDvrPointers[i];

                if (ptr >= buffer.size()) {
                    continue;
                }

                hasDvrPacketsLeft = true;
                const auto packetMs = av_rescale_q(buffer[ptr].pts, (i == 0) ? _vcodecContext->time_base : _acodecContext->time_base, AVRational{1, 1000});
                if (packetMs < smallestMs) {
                    smallestMs = packetMs;
                    selectedStream = i;
                }
            }

            if (!hasDvrPacketsLeft || selectedStream == _avcontext->nb_streams) {
                break;
            }

            // Copy the selected packet (as indicated by the pointer) from the selected stream.
            const auto& p = _dvrBuffer[selectedStream][currentDvrPointers[selectedStream]++];
            AVPacket* dupPacket = av_packet_clone(&p);
            if (!dupPacket) {
                continue;
            }

            addPacketToFileOutput(*dupPacket);
            av_packet_unref(dupPacket);
        }

        const auto realStartTimeUnixTime = _syncStartTime + std::chrono::milliseconds(av_rescale_q(_streamPtsOffset[0], _vcodecContext->time_base, AVRational{1, 1000}));
        retVodStartTime = shared::convertClockTime<service::recorder::encoder::AVSyncClock::time_point, shared::TimePoint>(realStartTimeUnixTime);
    } else {
        retVodStartTime = shared::nowUtc();
    }
    _fileOutputReady = true;
    return retVodStartTime;
}

void FfmpegAvEncoderImpl::start() {
    LOG_INFO("Request FFmpeg encoder start." << std::endl);
    std::lock_guard<std::mutex> guard(_startMutex);

    if (!_canStart) {
        LOG_INFO("Aborting start due to existing stop call." << std::endl);
    }

    if (!_vstream) {
        return;
    }

    // This must be set before _running is set to true as addVideo/AudioFrame will use this value and
    // use this time to sync the audio and video. This time is when our video and audio should start.
    _syncStartTime = AVSyncClock::now();
    for (const auto& s : _astreams) {
        s->nextPacketTime = _syncStartTime;
    }

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
        std::lock_guard<std::shared_mutex> guard(_runningMutex);
        if (_running) {
            LOG_INFO("...Ignoring 2nd start call on FFmpeg encoder." << std::endl);
            return;
        }
        _running = true;
    }

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

    LOG_INFO("Finish start FFmpeg encoder." << std::endl);
}

void FfmpegAvEncoderImpl::flushPacketQueue() {
    while ((_fileOutputReady || _dvrBufferReady) && !_packetQueue.empty()) {
        AVPacket packet = _packetQueue.front();
        _packetQueue.pop_front();

        if (_dvrBufferReady) {
            addPacketToDvrBuffer(packet);
        }

        if (_fileOutputReady) {
            addPacketToFileOutput(packet);
        }

        av_packet_unref(&packet);
    }
}

void FfmpegAvEncoderImpl::addPacketToFileOutput(AVPacket& packet) {
    int ret = 0;

    const auto offset = _streamPtsOffset[packet.stream_index];
    // We do the PTS/DTS fuckery caused by the DVR buffer here and not in the encoding.
    // Because we know to a certain degree that this particular code won't get called until
    // the proper pts offset is set and we won't have a weird random spike in the PTS.
    packet.pts -= offset;
    packet.dts -= offset;

    // Furthermore, we assume packets come in in the timebase of the codec. We need to conver that
    // to the time base of the stream before we write the frame.
    AVRational codecTimebase;
    AVRational streamTimebase = _avcontext->streams[packet.stream_index]->time_base;
    if (packet.stream_index == 0) {
        codecTimebase = _vcodecContext->time_base;
    } else {
        codecTimebase = _acodecContext->time_base;
    }

    av_packet_rescale_ts(&packet, codecTimebase, streamTimebase);

    if ((ret = av_interleaved_write_frame(_avcontext, &packet)) < 0) {
        char errBuff[2048];
        av_make_error_string(errBuff, 2048, ret);
        LOG_ERROR("Failed to write packet: " << errBuff << std::endl);
    }
}

void FfmpegAvEncoderImpl::addPacketToDvrBuffer(const AVPacket& packet) {
    std::lock_guard guard(_dvrMutex);

    AVPacket* dvrPacket = av_packet_clone(&packet);
    if (!dvrPacket) {
        return;
    }

    auto& buffer = _dvrBuffer[dvrPacket->stream_index];
    buffer.push_back(*dvrPacket);

    // Check to see if the amount of time that elapsed between the latest packet
    // and the first packet is larger than the max. If so, keep popping off until it's not.
    while (isDvrBufferLongerThanMaxSeconds(buffer, packet.stream_index)) {
        AVPacket front = buffer.front();
        buffer.pop_front();
        av_packet_unref(&front);
    }
}

bool FfmpegAvEncoderImpl::isDvrBufferLongerThanMaxSeconds(const std::deque<AVPacket>& buffer, int streamIndex) const {
    if (buffer.empty()) {
        return false;
    }

    const auto ptsDiff = buffer.back().pts - buffer.front().pts;

    // Need to convert the PTS difference to seconds since it'll depend on the stream's framerate.
    if (streamIndex < 0 || static_cast<unsigned int>(streamIndex) >= _avcontext->nb_streams) {
        return false;
    }

    AVRational timeBase = (streamIndex == 0) ? _vcodecContext->time_base : _acodecContext->time_base;

    // Convert the time first into milliseconds so we get some more precision before doing the comparison
    // in the double space for seconds.
    const auto diffMs = av_rescale_q(ptsDiff, timeBase, AVRational{1, 1000});
    return ((diffMs / 1000.0) > _maxDvrBufferTimeSeconds);
}

bool FfmpegAvEncoderImpl::hasAudioFrameAvailable() const {
    for (const auto& stream : _astreams) {
        std::shared_lock<std::shared_mutex> guard(stream->graphMutex);
        if (stream->sinkFrames.empty()) {
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
        std::lock_guard<std::shared_mutex> guard(_runningMutex);
        _running = false;
    }

    LOG_INFO("Waiting for IO thread to finish [" << _running << "]..." << std::endl);
    if (_packetThread.joinable()) {
        LOG_INFO("\tJoining packet thread..." << std::endl);
        _packetThread.join();
    }

    LOG_INFO("SquadOV FFMpeg Encoder Stats: " << std::endl
        << "\tVideo Frames: [Receive: " << _receivedVideoFrames  << "] [Process: " << _processedVideoFrames << "]" << std::endl
    );

    const auto numSecondsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(AVSyncClock::now() - _syncStartTime).count() * 1.0e-3;
    const auto estimatedFps = _receivedVideoFrames / numSecondsElapsed;
    LOG_INFO("\tEstimated FPS: " << estimatedFps << std::endl);

    for (size_t s = 0; s < _astreams.size(); ++s) {
        LOG_INFO("\tAudio Samples [" << s 
            << "]: Receive - " << _astreams[s]->receivedSamples
            << ":: Process - " << _astreams[s]->processedSamples
            << ":: Converted - " << _astreams[s]->convertedSamples
            << ":: Encoded - " << _astreams[s]->encodedSamples
        << std::endl);
    }

    // Flush packets from encoder. Don't do this for AMD's encoder when GPU encoding
    // because something is wrong there......
    if (_doPostVideoFlush && _d3d) {
        auto immediate = _d3d->immediateContext();
        encode(_vcodecContext, nullptr, _vstream);
    }

    if (!!_acodecContext) {
        encode(_acodecContext, nullptr, _astream);
    }

    flushPacketQueue();

    _fileOutputReady = false;
    _dvrBufferReady = false;

    for (auto& buffer: _dvrBuffer) {
        for (auto& p: buffer) {
            av_packet_unref(&p);
        }
    }
    _dvrBuffer.clear();
    _streamPtsOffset.clear();
    
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

#ifdef _WIN32
void FfmpegAvEncoder::addVideoFrame(ID3D11Texture2D* image, size_t numFrames) {
    _impl->addVideoFrame(image, numFrames);
}
#endif

void FfmpegAvEncoder::initializeVideoStream(const service::renderer::D3d11SharedContextPtr& d3d, const service::system::RecordingSettings& settings, size_t width, size_t height) {
    _impl->initializeVideoStream(d3d, settings, width, height);
}

VideoStreamContext FfmpegAvEncoder::getVideoStreamContext() const {
    return _impl->getVideoStreamContext();
}

shared::TimePoint FfmpegAvEncoder::open(const std::string& outputUrl, std::optional<AVSyncClock::time_point> dvrFillInStartTime) {
    return _impl->open(outputUrl, dvrFillInStartTime);
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

void FfmpegAvEncoder::finalizeStreams() {
    _impl->finalizeStreams();
}

void FfmpegAvEncoder::resizeDvrBuffer(double timeSeconds) {
    _impl->resizeDvrBuffer(timeSeconds);
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

size_t FfmpegAvEncoder::addAudioInput(const service::recorder::audio::AudioPacketProperties& inputProps, const AudioInputSettings& settings) {
    return _impl->addAudioInput(inputProps, settings);
}

void FfmpegAvEncoder::addAudioFrame(const service::recorder::audio::FAudioPacketView& view, size_t encoderIdx, const AVSyncClock::time_point& tm) {
    _impl->addAudioFrame(view, encoderIdx, tm);
}

AVSyncClock::time_point FfmpegAvEncoder::getSyncStartTime() const {
    return _impl->getSyncStartTime();
}

}