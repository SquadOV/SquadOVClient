#include "recorder/video/ffmpeg_video_encoder.h"

#include "shared/errors/error.h"

#include <cstdio>
#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>

#include "recorder/video/ffmpeg_utils.h"
}

#define DEBUG_IMAGES 0
#define LOG_FRAME_TIME 0

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

    void initialize(size_t fps, size_t width, size_t height);
    void addVideoFrame(const service::recorder::image::Image& frame);

    void start();
    void stop();
private:
    void swapBuffers();

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

    // Front and back buffering to support constant frame rate videos.
    std::thread _encodingThread;
    bool _running = false;
    std::chrono::milliseconds _msPerFrame;

    std::unique_ptr<service::recorder::image::Image> _frontBuffer;

    std::mutex _backMutex;
    std::unique_ptr<service::recorder::image::Image> _backBuffer;
    bool _backBufferDirty = false;
};

FfmpegVideoEncoderImpl::FfmpegVideoEncoderImpl(const std::filesystem::path& path):
    _path(path) {
    _path.replace_extension(std::filesystem::path(".mp4"));

    _avformat = av_guess_format(nullptr, _path.string().c_str(), nullptr);
    if (!_avformat) {
        THROW_ERROR("Failed to find the AV format for:" << path.extension().string());
    }

    if (avformat_alloc_output_context2(&_avcontext, _avformat, nullptr, _path.string().c_str()) < 0) {
        THROW_ERROR("Failed to allocate AV context.");
    }
}

FfmpegVideoEncoderImpl::~FfmpegVideoEncoderImpl() {
    sws_freeContext(_sws);
    avio_closep(&_avcontext->pb);
    av_frame_free(&_vframe);
    avcodec_free_context(&_vcodecContext);
    avformat_free_context(_avcontext);
}

void FfmpegVideoEncoderImpl::initialize(size_t fps, size_t width, size_t height) {
    // Try to use hardware encoding first. If not fall back on mpeg4.
    const std::string encodersToUse[] = {
        "h264_amf",
        "h264_nvenc",
        "mpeg4"
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
            _vcodecContext->thread_count = 16;

            // We're going to specify time in ms (instead of frames) so set the timebase to 1ms.
            _vcodecContext->time_base = AVRational{1, static_cast<int>(fps)};
            _vcodecContext->framerate = AVRational{static_cast<int>(fps), 1};

            // This can be high since we're only dealing with local videos for now.
            _vcodecContext->gop_size = 10;
            _vcodecContext->max_b_frames = 1;
            
            // Actually initialize the codec context.
            AVDictionary *options = nullptr;

            if (_vcodecContext->codec_id == AV_CODEC_ID_H264) {
                av_dict_set(&options, "preset", "medium", 0);
            }

            const auto ret = avcodec_open2(_vcodecContext, _vcodec, &options);
            if (ret < 0) {
                THROW_ERROR("Failed to initialize video context");
            }

            foundEncoder = true;
            std::cout << "FFmpeg Found Encoder: " << enc << std::endl;
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
    
    _frontBuffer = std::make_unique<recorder::image::Image>();
    _backBuffer = std::make_unique<recorder::image::Image>();

    _frontBuffer->initializeImage(width, height);
    _backBuffer->initializeImage(width, height);
    _msPerFrame = std::chrono::milliseconds(static_cast<size_t>(1.0 / fps * 1000.0));
}

void FfmpegVideoEncoderImpl::addVideoFrame(const service::recorder::image::Image& frame) {
#if DEBUG_IMAGES
    {
        std::cout << "HANDLE IMAGE: " << frameMs << std::endl;
        std::ostringstream imgFname;
        imgFname << _path.string() << "_" << frameMs << ".png";
        frame.saveToFile(std::filesystem::path(imgFname.str()));
    }
#endif

    std::unique_lock<std::mutex> guard(_backMutex);
    _backBuffer->copyFrom(frame);
    _backBufferDirty = true;
}

void FfmpegVideoEncoderImpl::swapBuffers() {
    std::unique_lock<std::mutex> guard(_backMutex);
    if (!_backBufferDirty) {
        return;
    }
    _frontBuffer.swap(_backBuffer);
    _backBufferDirty = false;
}

void FfmpegVideoEncoderImpl::start() {
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
    _encodingThread = std::thread([this](){
        const auto start = std::chrono::high_resolution_clock::now();
        auto refFrameTime = start;
        int64_t frameNum = 0;
        
        while (_running) {
            const auto now = std::chrono::high_resolution_clock::now();
            bool skipFrame = false;

            // If "now" is before the refFrameTime then we want to wait until
            // refFrameTime. But if "now" is after refFrameTime then we skip a frame
            // to ensure that we stay on track.
            if (now < refFrameTime) {
                std::this_thread::sleep_until(refFrameTime);
            } else {
                skipFrame = true;
            }

#if LOG_FRAME_TIME
            const auto taskNow = std::chrono::high_resolution_clock::now();
#endif

            // These two should happen regardless of whether or not we skip a frame.
            // refFrameTime needs to be incremented because we want to write to the next frame.
            // Need to set vframe pts to what the frame would have already been so that the frameNum
            // counter keeps updating so when the frame is no longer skipped, we're inserting the data
            // at the right frame. swapBuffers should be called because we should want the latest of
            // whatever the user has written in the front buffer.
            refFrameTime += _msPerFrame;
            _vframe->pts = frameNum++;
            swapBuffers();

            if (skipFrame) {
                continue;
            }

            if (av_frame_make_writable(_vframe) < 0) {
                THROW_ERROR("Failed to make video frame writeable.");
            }

            // Convert the RGB image to YUV for encoding.
            const auto* buffer = _frontBuffer->buffer();
            const int bufferStride[1] = { static_cast<int>(_frontBuffer->width() * _frontBuffer->bytesPerPixel()) };
            sws_scale(_sws, &buffer, bufferStride, 0, static_cast<int>(_frontBuffer->height()), _vframe->data, _vframe->linesize);
            encode(_vcodecContext, _avcontext, _vframe, _vstream);
        
#if LOG_FRAME_TIME
            const auto elapsedTime = std::chrono::high_resolution_clock::now() - taskNow;
            const auto numMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();
            std::cout << "FFMPEG Encode: " << numMs << "ms" << std::endl;
#endif
        }
    });
}

void FfmpegVideoEncoderImpl::stop() {
    if (!_running) {
        return;
    }

    _running = false;
    _encodingThread.join();

    // Flush packets from encoder.
    encode(_vcodecContext, _avcontext, nullptr, _vstream);
    av_write_trailer(_avcontext);
}

FfmpegVideoEncoder::FfmpegVideoEncoder(const std::filesystem::path& path):
    VideoEncoder(path),
    _impl(new FfmpegVideoEncoderImpl(path)) {  
}

FfmpegVideoEncoder::~FfmpegVideoEncoder() = default;

void FfmpegVideoEncoder::addVideoFrame(const service::recorder::image::Image& frame) {
    _impl->addVideoFrame(frame);
}

void FfmpegVideoEncoder::initialize(size_t fps, size_t width, size_t height) {
    _impl->initialize(fps, width, height);
}

void FfmpegVideoEncoder::start() {
    _impl->start();
}

void FfmpegVideoEncoder::stop() {
    _impl->stop();
}

}