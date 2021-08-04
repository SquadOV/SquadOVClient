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
    void initializeVideoStream(const service::system::RecordingSettings& settings, size_t width, size_t height, const service::renderer::D3d11OverlayRendererPtr& overlay);
    void setOverlayRenderer(const service::renderer::D3d11OverlayRendererPtr& renderer);
    VideoStreamContext getVideoStreamContext() const { return _videoStreamContext; }
    void addVideoFrame(const service::recorder::image::Image& frame);
#ifdef _WIN32
    void addVideoFrame(ID3D11Texture2D* image);
#endif

    void getVideoDimensions(size_t& width, size_t& height);
    service::recorder::image::Image getFrontBuffer() const;

    void initializeAudioStream();
    size_t addAudioInput(const service::recorder::audio::AudioPacketProperties& inputProps, const AudioInputSettings& settings);
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
    
    AVSyncClock::time_point _syncStartTime;
    // Video Output
    AVStream* _vstream = nullptr;
    const AVCodec* _vcodec = nullptr;
    AVCodecContext* _vcodecContext = nullptr;
    int64_t _vFrameNum = 0;
    long long numVideoFramesToEncode();
    void videoSwapAndEncode();

    // Audio Output
    const AVCodec* _acodec = nullptr;
    AVCodecContext* _acodecContext = nullptr;
    AVStream* _astream = nullptr;
    AVFrame* _aframe = nullptr;
    int64_t _aFrameNum = 0;

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
    bool _useVfr3 = false;

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
    service::renderer::D3d11SharedContext* d3d = service::renderer::getSharedD3d11Context();
    auto immediate = d3d->immediateContext();

    avcodec_free_context(&_vcodecContext);
    av_frame_free(&_aframe);
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

        av_packet_rescale_ts(&packet, cctx->time_base, st->time_base);
        packet.stream_index = st->index;

        if (packet.dts > packet.pts) {
            // Possible if there's a jump in the pts.
            packet.pts = packet.dts;
        }

        if (!packet.duration && frame && frame->pkt_duration) {
            packet.duration = frame->pkt_duration;
        }

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

void FfmpegAvEncoderImpl::initializeVideoStream(const service::system::RecordingSettings& settings, size_t width, size_t height, const service::renderer::D3d11OverlayRendererPtr& overlay) {
    // Try to use hardware encoding first. If not fall back on mpeg4.
    struct EncoderChoice {
        std::string name;
        VideoStreamContext ctx;
        bool isGpuEncoder;
    };

    const EncoderChoice encodersToUse[] = {
        {"h264_nvenc", VideoStreamContext::GPU, true },
        {"h264_amf", VideoStreamContext::GPU, true },
        {"h264_mf", VideoStreamContext::CPU, true },
        {"libopenh264", VideoStreamContext::CPU, false }
    };

    service::renderer::D3d11SharedContext* d3d = service::renderer::getSharedD3d11Context();
    const auto canUseGpu = FfmpegGPUVideoSwapChain::isSupported(d3d, width, height);
    auto immediate = d3d->immediateContext();

    bool foundEncoder = false;
    bool canUseHwAccel = false;
    for (const auto& enc : encodersToUse) {
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

            canUseHwAccel = (enc.ctx == VideoStreamContext::GPU) && canUseGpu && settings.useVideoHw;
            _vcodecContext->pix_fmt = canUseHwAccel ? AV_PIX_FMT_D3D11 : AV_PIX_FMT_YUV420P;
            _vcodecContext->bit_rate = std::min(settings.bitrateKbps, int64_t(6000)) * 1000;
            _vcodecContext->rc_max_rate = _vcodecContext->bit_rate;
            _vcodecContext->rc_buffer_size = _vcodecContext->rc_max_rate * 2;
            _vcodecContext->thread_count = 0;
            _vcodecContext->max_b_frames = 3;
            
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

                // Not sure why but this having this option > 0 causes issues using the AMD AMF encoder.
                if (enc.name == "h264_amf") {
                    _vcodecContext->max_b_frames = 0;
                    _doPostVideoFlush = false;
                }
            }

            // We're going to specify time in ms (instead of frames) so set the timebase to 1ms.
            _vcodecContext->time_base = AVRational{1, static_cast<int>(settings.fps)};
            _vcodecContext->framerate = AVRational{static_cast<int>(settings.fps), 1};

            // I think we get better video quality with a larger GOP size? Not sure tbh. I saw some flickering in certain formats
            // if the GOP size is too small.
            if (strcmp(_avformat->name, "hls") == 0) {
                _vcodecContext->gop_size = static_cast<int>(settings.fps * 1);
            } else {
                _vcodecContext->gop_size = static_cast<int>(settings.fps * 5);
            }

            if (_avcontext->oformat->flags & AVFMT_GLOBALHEADER) {
                _vcodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            }
            
            // Actually initialize the codec context.
            AVDictionary *options = nullptr;

            if (_vcodecContext->codec_id == AV_CODEC_ID_H264) {
                av_dict_set(&options, "preset", "medium", 0);

                // H264 Profile
                if (enc.name == "h264_mf") {
                    _vcodecContext->profile = FF_PROFILE_H264_HIGH;
                } else {
                    av_dict_set(&options, "profile", "high", 0);
                }

                if (!settings.useBitrate) {
                    // CRF/CQ
                    if (enc.name == "h264_mf") {
                        av_dict_set(&options, "quality", "30", 0);
                    } else if (enc.name == "h264_nvenc") {
                        av_dict_set(&options, "qp", "30", 0);
                    } else if (enc.name == "h264_amf") {
                        av_dict_set(&options, "qp_i", "30", 0);
                        av_dict_set(&options, "qp_p", "30", 0);
                        av_dict_set(&options, "qp_b", "30", 0);
                    } else if (enc.name == "libopenh264") {
                        _vcodecContext->qmin = 20;
                        _vcodecContext->qmax = 30;
                    }

                    // Rate control
                    if (enc.name == "h264_mf") {
                        av_dict_set(&options, "rate_control", "quality", 0);
                    } else if (enc.name == "h264_nvenc") {
                        av_dict_set(&options, "rc", "constqp", 0);
                    } else if (enc.name == "h264_amf") {
                        av_dict_set(&options, "rc", "cqp", 0);
                    } else if (enc.name == "libopenh264") {
                        av_dict_set(&options, "rc_mode", "quality", 0);
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
    _videoSwapChain->initializeGpuSupport(d3d);
    _videoSwapChain->initializeOverlay(overlay);
    _videoSwapChain->initialize(_vcodecContext, _vcodecContext->hw_frames_ctx);
    _fps = settings.fps;
    _useVfr3 = settings.useVfr3;
    _nsPerFrame = std::chrono::nanoseconds(static_cast<size_t>(1.0 / settings.fps * 1.0e+9));
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

void FfmpegAvEncoderImpl::addVideoFrame(const service::recorder::image::Image& frame) {
    std::shared_lock<std::shared_mutex> guard(_runningMutex);
    if (!_running) {
        return;
    }
    
    if (!numVideoFramesToEncode()) {
        return;
    }

    _videoSwapChain->receiveCpuFrame(frame);

    if (_running) {
        ++_receivedVideoFrames;
    }

    videoSwapAndEncode();
}

void FfmpegAvEncoderImpl::addVideoFrame(ID3D11Texture2D* image) {
    std::shared_lock<std::shared_mutex> guard(_runningMutex);
    if (!_running) {
        return;
    }

    if (!numVideoFramesToEncode()) {
        return;
    }

    _videoSwapChain->receiveGpuFrame(image);
    
    if (_running) {
        ++_receivedVideoFrames;
    }

    videoSwapAndEncode();
}

long long FfmpegAvEncoderImpl::numVideoFramesToEncode() {
    // This gets called every time we receive a video frame. In the case where the frame is too early
    // to be considered the next frame, we just do nothing (encode 0 frames). In the case where too much
    // time has elapsed, then it's possible that we want to encode *multiple* frames. In that case
    // we use the same incoming frame multiple times and just have duplicate frames in the output.
    const auto refFrameTime = _syncStartTime + std::chrono::nanoseconds(_nsPerFrame * _vFrameNum);
    const auto now = AVSyncClock::now();

    if (now < refFrameTime) {
        return 0;
    } else {
        // We should note that refFrameTime is the time at which we should add a new frame. Thus, any time PAST refFrameTime indicates that
        // at least 1 frame should be added. Thus, we want to use a CEIL instead of a FLOOR here to represent that fact.
        const auto elapsedFrames = std::ceil(static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(now - refFrameTime).count()) / _nsPerFrame.count());
        return static_cast<long long>(elapsedFrames);
    }
}

void FfmpegAvEncoderImpl::videoSwapAndEncode() {
    // Get this number again just in case it took us awhile to receive the frame.
    const auto numFramesToEncode = numVideoFramesToEncode();
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

    _videoSwapChain->swap();

    AVFrame* frame = _videoSwapChain->getFrontBufferFrame();
    if (_videoSwapChain->hasValidFrontBuffer()) {
        ++_processedVideoFrames;
    } else {
        LOG_WARNING("Invalid front buffer...ignoring." << std::endl);
        return;
    }

    bool forceFrame0 = false;
    const int64_t duration = desiredFrameNum - _vFrameNum;
    if (_useVfr3) {
        forceFrame0 = (_vFrameNum == 0 && desiredFrameNum > 1);
        _vFrameNum = desiredFrameNum - 1;
    }

    service::renderer::D3d11SharedContext* d3d = service::renderer::getSharedD3d11Context();
    auto immediate = d3d->immediateContext();

    if (forceFrame0) {
        frame->pts = 0;
        frame->pkt_duration = 1;
        encode(_vcodecContext, frame, _vstream);
    }

    for (; _vFrameNum < desiredFrameNum; ++_vFrameNum) {
        frame->pts = _vFrameNum;
        frame->pkt_duration = duration;
        encode(_vcodecContext, frame, _vstream);
    }
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

service::recorder::image::Image FfmpegAvEncoderImpl::getFrontBuffer() const {
    return _videoSwapChain->cpuCopyFrontBuffer();
}

void FfmpegAvEncoderImpl::open() {
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
    }

    if (avformat_write_header(_avcontext, &headerOptions) < 0) {
        THROW_ERROR("Failed to write header");
    }
    av_dict_free(&headerOptions);
    av_dump_format(_avcontext, 0, _streamUrl.c_str(), 1);
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
    if (_doPostVideoFlush) {
        service::renderer::D3d11SharedContext* d3d = service::renderer::getSharedD3d11Context();
        auto immediate = d3d->immediateContext();

        encode(_vcodecContext, nullptr, _vstream);
    }

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

void FfmpegAvEncoder::initializeVideoStream(const service::system::RecordingSettings& settings, size_t width, size_t height, const service::renderer::D3d11OverlayRendererPtr& overlay) {
    _impl->initializeVideoStream(settings, width, height, overlay);
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

size_t FfmpegAvEncoder::addAudioInput(const service::recorder::audio::AudioPacketProperties& inputProps, const AudioInputSettings& settings) {
    return _impl->addAudioInput(inputProps, settings);
}

void FfmpegAvEncoder::addAudioFrame(const service::recorder::audio::FAudioPacketView& view, size_t encoderIdx, const AVSyncClock::time_point& tm) {
    _impl->addAudioFrame(view, encoderIdx, tm);
}

}