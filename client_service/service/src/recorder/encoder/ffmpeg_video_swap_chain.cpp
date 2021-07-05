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
    if (_frontSws) {
        sws_freeContext(_frontSws);
        _frontSws = nullptr;
    }

    if (_backSws) {
        sws_freeContext(_backSws);
        _backSws = nullptr;
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

    _overlay->initializeGpu();
    _frontBuffer = std::make_unique<service::recorder::image::Image>();
    _backBuffer = std::make_unique<service::recorder::image::Image>();
}

service::recorder::image::Image FfmpegCPUVideoSwapChain::cpuCopyFrontBuffer() const {
    std::lock_guard<std::mutex> guard(_frontMutex);
    service::recorder::image::Image image;
    if (_frontBuffer->isInit()) {
        image.initializeImage(_frontBuffer->width(), _frontBuffer->height());
        image.copyFrom(*_frontBuffer);
    }
    return image;
}

bool FfmpegCPUVideoSwapChain::hasValidFrontBuffer() const {
    std::lock_guard<std::mutex> fguard(_frontMutex);
    return _frontBuffer->isInit();
}

void FfmpegCPUVideoSwapChain::swap() {
    std::lock_guard<std::mutex> fguard(_frontMutex);
    std::lock_guard<std::mutex> bguard(_backMutex);
    if (!_backBufferDirty) {
        return;
    }
    _frontBuffer.swap(_backBuffer);
    std::swap(_frontSws, _backSws);
    _backBufferDirty = false;
}

AVFrame* FfmpegCPUVideoSwapChain::getFrontBufferFrame() {
    std::lock_guard<std::mutex> guard(_frontMutex);
    if (_frontBuffer->isInit()) {
        const auto* inputBuffer = _frontBuffer->buffer();
        const int inputBufferStride[1] = { static_cast<int>(_frontBuffer->width() * _frontBuffer->bytesPerPixel()) };
        sws_scale(_frontSws, &inputBuffer, inputBufferStride, 0, static_cast<int>(_frontBuffer->height()), _frame->data, _frame->linesize);
    }
    return _frame;
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
    if (_backBuffer->width() != width || _backBuffer->height() != height) {
        _backBuffer->initializeImage(width, height);

        if (!!_backSws) {
            sws_freeContext(_backSws);
            _backSws = nullptr;
        }

        _backSws = sws_getContext(
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

        if (!_backSws) {
            THROW_ERROR("Failed to create SWS context.");
        }

        _overlay->reinitializeRenderer(width, height);
    }
}

void FfmpegCPUVideoSwapChain::receiveCpuFrame(const service::recorder::image::Image& frame) {
    std::lock_guard<std::mutex> guard(_backMutex);
    reinitBackBuffer(frame.width(), frame.height());

    if (_overlay) {
        _overlay->render(_backBuffer, frame);
    } else {
        _backBuffer->copyFrom(frame);
    }
    _backBufferDirty = true;
}

#ifdef _WIN32
void FfmpegCPUVideoSwapChain::receiveGpuFrame(ID3D11Texture2D* frame) {
    if (!isGpuInit()) {
        THROW_ERROR("Swap chain not initialized for GPU usage.");
    }

    D3D11_TEXTURE2D_DESC desc;
    frame->GetDesc(&desc);

    std::lock_guard<std::mutex> guard(_backMutex);
    reinitBackBuffer(desc.Width, desc.Height);

    if (_overlay) {
        _overlay->render(_backBuffer, frame, _shared);
    } else {
        auto immediate = _shared->immediateContext();
        _backBuffer->loadFromD3d11TextureWithStaging(_shared->device(), immediate.context(), frame);
    }
    _backBufferDirty = true;
}
#endif

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

    _overlay->initializeGpu();
    _processor = std::make_unique<service::renderer::D3d11VideoProcessor>(_shared);
    _processor->setFfmpegColorspace(_frame->colorspace);
    _frontBuffer = std::make_unique<service::recorder::image::D3dImage>(_shared);
    _backBuffer = std::make_unique<service::recorder::image::D3dImage>(_shared);
}

AVFrame* FfmpegGPUVideoSwapChain::getFrontBufferFrame() {
    std::lock_guard<std::mutex> guard(_frontMutex);

    if (_frontBuffer->isInit()) {
        // Copy from the front buffer to the frame's texture using D3D11's video processing capabilities
        // to ensure that we're able to convert to NV12 properly.
        auto* frameTexture = reinterpret_cast<ID3D11Texture2D*>(_frame->data[0]);
        _processor->process(_frontBuffer->rawTexture(), frameTexture);
    }

    return _frame;
}

void FfmpegGPUVideoSwapChain::swap() {
    std::lock_guard<std::mutex> fguard(_frontMutex);
    std::lock_guard<std::mutex> bguard(_backMutex);
    if (!_backBufferDirty) {
        return;
    }
    _frontBuffer.swap(_backBuffer);
    _backBufferDirty = false;
}

service::recorder::image::Image FfmpegGPUVideoSwapChain::cpuCopyFrontBuffer() const {
    std::lock_guard<std::mutex> fguard(_frontMutex);
    return _frontBuffer->cpuImage();
}

bool FfmpegGPUVideoSwapChain::hasValidFrontBuffer() const {
    std::lock_guard<std::mutex> fguard(_frontMutex);
    return _frontBuffer->isInit();
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
    if (_backBuffer->width() != width || _backBuffer->height() != height) {
        _backBuffer->initializeImage(width, height, true);
        _overlay->reinitializeRenderer(width, height);
    }
}

void FfmpegGPUVideoSwapChain::receiveCpuFrame(const service::recorder::image::Image& frame) {
    std::lock_guard<std::mutex> guard(_backMutex);
    reinitBackBuffer(frame.width(), frame.height());

    if (_overlay) {
        _overlay->render(_backBuffer, frame);
    } else {
        _backBuffer->copyFromCpu(frame);
    }
    _backBufferDirty = true;
}

#ifdef _WIN32
void FfmpegGPUVideoSwapChain::receiveGpuFrame(ID3D11Texture2D* frame) {
    D3D11_TEXTURE2D_DESC desc;
    frame->GetDesc(&desc);

    std::lock_guard<std::mutex> guard(_backMutex);
    reinitBackBuffer(desc.Width, desc.Height);
    if (_overlay) {
        _overlay->render(_backBuffer, frame, _shared);
    } else {
        _backBuffer->copyFromGpu(frame);
    }
    _backBufferDirty = true;
}
#endif

}