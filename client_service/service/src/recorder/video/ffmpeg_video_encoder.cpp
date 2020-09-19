#include "recorder/video/ffmpeg_video_encoder.h"

#include "shared/errors/error.h"

#include <cstdio>
#include <iostream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>

#include "recorder/video/ffmpeg_utils.h"
}

#define DEBUG_IMAGES 0

namespace service::recorder::video {
namespace {

void encode(AVCodecContext* cctx, AVFormatContext* fctx, AVFrame* frame, AVStream * st){
    // Encode video frame.
    if (avcodec_send_frame(cctx, frame) < 0) {
        THROW_ERROR("Failed to send video frame.");
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
        if (av_interleaved_write_frame(fctx, &packet) < 0) {
            THROW_ERROR("Failed to write frame");
        }
        av_packet_unref(&packet);
    }
}

}

class FfmpegVideoEncoderImpl {
public:
    FfmpegVideoEncoderImpl(const std::filesystem::path& path);
    ~FfmpegVideoEncoderImpl();

    void initialize(size_t width, size_t height);
    void addVideoFrame(long long frameMs, const service::recorder::image::Image& frame);
private:
    std::filesystem::path _path;

    // AV Output
    AVOutputFormat* _avformat = nullptr;
    AVFormatContext* _avcontext = nullptr;
    
    // Video Output
    AVStream* _vstream = nullptr;
    const AVCodec* _vcodec = nullptr;
    AVCodecContext* _vcodecContext = nullptr;
    AVFrame* _vframe = nullptr;
    SwsContext* _sws = nullptr;
};

FfmpegVideoEncoderImpl::FfmpegVideoEncoderImpl(const std::filesystem::path& path):
    _path(path) {

    _avformat = av_guess_format(nullptr, _path.string().c_str(), nullptr);
    if (!_avformat) {
        THROW_ERROR("Failed to find the AV format for:" << path.extension().string());
    }

    if (avformat_alloc_output_context2(&_avcontext, _avformat, nullptr, _path.string().c_str()) < 0) {
        THROW_ERROR("Failed to allocate AV context.");
    }

    // Force us to use VP8 for speed although slightly worse quality.
    _vcodec = avcodec_find_encoder_by_name("libvpx");
    if (!_vcodec) {
        THROW_ERROR("Failed to find the video codec.");
    }

    _vstream = avformat_new_stream(_avcontext, _vcodec);
    if (!_vstream) {
        THROW_ERROR("Failed to create video stream.");
    }

    _vcodecContext = avcodec_alloc_context3(_vcodec);
    if (!_vcodecContext) {
        THROW_ERROR("Failed to allocate video context.");
    }
}

FfmpegVideoEncoderImpl::~FfmpegVideoEncoderImpl() {
    // Flush packets from encoder.
    encode(_vcodecContext, _avcontext, nullptr, _vstream);
    av_write_trailer(_avcontext);

    sws_freeContext(_sws);
    avio_closep(&_avcontext->pb);
    av_frame_free(&_vframe);
    avcodec_free_context(&_vcodecContext);
    avformat_free_context(_avcontext);
}

void FfmpegVideoEncoderImpl::initialize(size_t width, size_t height) {
    _vcodecContext->width = static_cast<int>(width);
    _vcodecContext->height = static_cast<int>(height);
    _vcodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    _vcodecContext->bit_rate = 6000000;
    _vcodecContext->thread_count = 16;

    // We're going to specify time in ms (instead of frames) so set the timebase to 1ms.
    _vcodecContext->time_base = AVRational{1, 1000};
    _vcodecContext->framerate = AVRational{0, 1};
    _vstream->time_base = _vcodecContext->time_base;

    // This can be high since we're only dealing with local videos for now.
    _vcodecContext->gop_size = 10;
    _vcodecContext->max_b_frames = 1;
    
    // Actually initialize the codec context.
    AVDictionary *options = nullptr;
    av_dict_set_int(&options, "deadline", 1, 0);
    av_dict_set_int(&options, "cpu-used", 8, 0);

    const auto ret = avcodec_open2(_vcodecContext, _vcodec, &options);
    if (ret < 0) {
        THROW_ERROR("Failed to initialize video context");
    }

    avcodec_parameters_from_context(_vstream->codecpar, _vcodecContext);

    if (_avcontext->oformat->flags & AVFMT_GLOBALHEADER) {
        _vcodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    // This is what actually gets us to write to a file.
    if (avio_open(&_avcontext->pb, _path.string().c_str(), AVIO_FLAG_WRITE) < 0) {
        THROW_ERROR("Failed to open video for output");
    }

    if (avformat_write_header(_avcontext, nullptr) < 0) {
        THROW_ERROR("Failed to write header");
    }

    av_dump_format(_avcontext, 0, _path.string().c_str(), 1);

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

    _sws = sws_getContext(
        _vframe->width,
        _vframe->height,
        AV_PIX_FMT_BGRA,
        _vframe->width,
        _vframe->height,
        AV_PIX_FMT_YUV420P,
        SWS_BICUBIC,
        nullptr,
        nullptr,
        nullptr);
    if (!_sws) {
        THROW_ERROR("Failed to create SWS context.");
    }
}

void FfmpegVideoEncoderImpl::addVideoFrame(long long frameMs, const service::recorder::image::Image& frame) {
    if (av_frame_make_writable(_vframe) < 0) {
        THROW_ERROR("Failed to make video frame writeable.");
    }
    _vframe->pts = frameMs;

#if DEBUG_IMAGES
    {
        std::cout << "HANDLE IMAGE: " << frameMs << std::endl;
        std::ostringstream imgFname;
        imgFname << _path.string() << "_" << frameMs << ".png";
        frame.saveToFile(std::filesystem::path(imgFname.str()));
    }
#endif

    // Convert the RGB image to YUV for encoding.
    const auto* buffer = frame.buffer();
    const int bufferStride[1] = { static_cast<int>(frame.width() * frame.bytesPerPixel()) };
    sws_scale(_sws, &buffer, bufferStride, 0, static_cast<int>(frame.height()), _vframe->data, _vframe->linesize);
    encode(_vcodecContext, _avcontext, _vframe, _vstream);
}

FfmpegVideoEncoder::FfmpegVideoEncoder(const std::filesystem::path& path):
    VideoEncoder(path),
    _impl(new FfmpegVideoEncoderImpl(path)) {  
}

FfmpegVideoEncoder::~FfmpegVideoEncoder() = default;

void FfmpegVideoEncoder::addVideoFrame(long long frameMs, const service::recorder::image::Image& frame) {
    _impl->addVideoFrame(frameMs, frame);
}

void FfmpegVideoEncoder::initialize(size_t width, size_t height) {
    _impl->initialize(width, height);
}

}