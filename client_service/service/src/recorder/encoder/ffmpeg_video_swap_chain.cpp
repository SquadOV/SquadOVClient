#include "recorder/encoder/ffmpeg_video_swap_chain.h"

#include "shared/errors/error.h"
#include "renderer/d3d11_texture.h"

#ifdef _WIN32
#include <Windows.h>
#include <VersionHelpers.h>
#endif

namespace service::recorder::encoder {

//
// CPU
//   

FfmpegCPUVideoSwapChain::~FfmpegCPUVideoSwapChain() {
    if (_sws) {
        sws_freeContext(_sws);
        _sws = nullptr;
    }

    if (_frame) {
        av_frame_free(&_frame);
        _frame = nullptr;
    }
}

void FfmpegCPUVideoSwapChain::initialize(AVCodecContext* context, AVBufferRef*) {
    _frame = av_frame_alloc();
    if (!_frame) {
        THROW_ERROR("Failed to allocate video frame.");
    }

    _frame->format = context->pix_fmt;
    _frame->width = context->width;
    _frame->height = context->height;
    _frame->pts = 0;

    if (av_frame_get_buffer(_frame, 0) < 0) {
        THROW_ERROR("Failed to get CPU frame buffer.");
    }

    _buffer = std::make_unique<service::recorder::image::Image>();
}

size_t FfmpegCPUVideoSwapChain::frameWidth() const {
    if (!_frame) {
        return 0;
    }
    return _frame->width;
}

size_t FfmpegCPUVideoSwapChain::frameHeight() const {
    if (!_frame) {
        return 0;
    }
    return _frame->height;
}

void FfmpegCPUVideoSwapChain::reinitBackBuffer(size_t width, size_t height) {
    if (_buffer->width() != width || _buffer->height() != height) {
        _buffer->initializeImage(width, height);

        if (!!_sws) {
            sws_freeContext(_sws);
            _sws = nullptr;
        }

        _sws = sws_getContext(
            static_cast<int>(width),
            static_cast<int>(height),
            AV_PIX_FMT_BGRA,
            _frame->width,
            _frame->height,
            AV_PIX_FMT_YUV420P,
            SWS_BICUBIC,
            nullptr,
            nullptr,
            nullptr
        );

        if (!_sws) {
            THROW_ERROR("Failed to create SWS context.");
        }
    }
}

AVFrame* FfmpegCPUVideoSwapChain::receiveCpuFrame(const service::recorder::image::Image& frame) {
    return process(frame);
}

#ifdef _WIN32
AVFrame* FfmpegCPUVideoSwapChain::receiveGpuFrame(ID3D11Texture2D* frame) {
    if (!isGpuInit()) {
        THROW_ERROR("Swap chain not initialized for GPU usage.");
    }

    D3D11_TEXTURE2D_DESC desc;
    frame->GetDesc(&desc);
    reinitBackBuffer(desc.Width, desc.Height);

    auto immediate = _shared->immediateContext();
    _buffer->loadFromD3d11TextureWithStaging(_shared->device(), immediate.context(), frame);
    return process(*_buffer);
}
#endif

AVFrame* FfmpegCPUVideoSwapChain::process(const service::recorder::image::Image& input) {
    if (!input.isInit() || !_sws) {
        return nullptr;
    }

    const auto* inputBuffer = input.buffer();
    const int inputBufferStride[1] = { static_cast<int>(input.width() * input.bytesPerPixel()) };
    sws_scale(_sws, &inputBuffer, inputBufferStride, 0, static_cast<int>(input.height()), _frame->data, _frame->linesize);
    return _frame;
}

//
// GPU
//

FfmpegGPUVideoSwapChain::~FfmpegGPUVideoSwapChain() {
    if (_frame) {
        av_frame_free(&_frame);
        _frame = nullptr;
    }
}

void FfmpegGPUVideoSwapChain::initialize(AVCodecContext* context, AVBufferRef* hwFrameCtx) {
    if (!hwFrameCtx) {
        THROW_ERROR("Cannot initialize GPU swap with nullptr hw frame context.");
    }

    if (!isGpuInit()) {
        THROW_ERROR("Cannot initialize GPU swap with no GPU context.");
    }

    _frame = av_frame_alloc();
    if (!_frame) {
        THROW_ERROR("Failed to allocate video frame.");
    }
    
    _frame->colorspace = context->colorspace;
    _frame->hw_frames_ctx = hwFrameCtx;
    _frame->format = context->sw_pix_fmt;
    _frame->width = context->width;
    _frame->height = context->height;
    _frame->pts = 0;

    if (av_hwframe_get_buffer(_frame->hw_frames_ctx, _frame, 0) < 0) {
        THROW_ERROR("Failed to get HW frame buffer.");
    }

    if (context->sw_pix_fmt != AV_PIX_FMT_NV12) {
        THROW_ERROR("Formats besides NV12 not supported.");
    }

    _processor = std::make_unique<service::renderer::D3d11VideoProcessor>(_shared);
    _processor->setFfmpegColorspace(_frame->colorspace);
    _buffer = std::make_unique<service::recorder::image::D3dImage>(_shared);
}

bool FfmpegGPUVideoSwapChain::isSupported(service::renderer::D3d11SharedContext* shared, size_t width, size_t height) {
    if (!IsWindows8OrGreater()) {
        return false;
    }

    // Create a temporary processor and do a check to make sure the device we're on supports
    // everything we'd expect.
    try {
        service::renderer::D3d11VideoProcessor processor(shared);
        return processor.isSupported(width, height);
    } catch (std::exception& ex) {
        LOG_WARNING("Critical failure in D3d11VideoProcessor: " << ex.what() << std::endl);
        return false;
    }
}

size_t FfmpegGPUVideoSwapChain::frameWidth() const {
    if (!_frame) {
        return 0;
    }
    return _frame->width;
}

size_t FfmpegGPUVideoSwapChain::frameHeight() const {
    if (!_frame) {
        return 0;
    }
    return _frame->height;
}

void FfmpegGPUVideoSwapChain::reinitBackBuffer(size_t width, size_t height) {
    if (_buffer->width() != width || _buffer->height() != height) {
        _buffer->initializeImage(width, height, true);
    }
}

AVFrame* FfmpegGPUVideoSwapChain::receiveCpuFrame(const service::recorder::image::Image& frame) {
    reinitBackBuffer(frame.width(), frame.height());

    _buffer->copyFromCpu(frame);
    return process(_buffer->rawTexture());
}

#ifdef _WIN32
AVFrame* FfmpegGPUVideoSwapChain::receiveGpuFrame(ID3D11Texture2D* frame) {
    return process(frame);
}
#endif

AVFrame* FfmpegGPUVideoSwapChain::process(ID3D11Texture2D* input) {
    if (!_processor) {
        return nullptr;
    }

    auto* frameTexture = reinterpret_cast<ID3D11Texture2D*>(_frame->data[0]);
    _processor->process(input, frameTexture);
    return _frame;
}

}