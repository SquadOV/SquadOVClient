#include "recorder/encoder/ffmpeg_video_swap_chain.h"

#include "shared/errors/error.h"

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

void FfmpegCPUVideoSwapChain::initialize(AVCodecContext* context) {
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

    _frontBuffer = std::make_unique<service::recorder::image::Image>();
    _backBuffer = std::make_unique<service::recorder::image::Image>();
}

service::recorder::image::Image FfmpegCPUVideoSwapChain::cpuCopyFrontBuffer() const {
    std::lock_guard<std::mutex> guard(_backMutex);
    service::recorder::image::Image image;
    if (_frontBuffer->isInit()) {
        image.initializeImage(_frontBuffer->width(), _frontBuffer->height());
        image.copyFrom(*_frontBuffer);
    }
    return image;
}

bool FfmpegCPUVideoSwapChain::hasValidFrontBuffer() const {
    return _frontBuffer->isInit();
}

void FfmpegCPUVideoSwapChain::swap() {
    std::lock_guard<std::mutex> guard(_backMutex);
    if (!_backBufferDirty) {
        return;
    }
    _frontBuffer.swap(_backBuffer);
    std::swap(_frontSws, _backSws);
    _backBufferDirty = false;
}

AVFrame* FfmpegCPUVideoSwapChain::getFrontBufferFrame() {
    std::lock_guard<std::mutex> guard(_backMutex);
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
    }
}

void FfmpegCPUVideoSwapChain::receiveCpuFrame(const service::recorder::image::Image& frame) {
    std::lock_guard<std::mutex> guard(_backMutex);
    reinitBackBuffer(frame.width(), frame.height());
    _backBuffer->copyFrom(frame);
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

    auto immediate = _shared->immediateContext();
    _backBuffer->loadFromD3d11TextureWithStaging(_shared->device(), immediate.context(), frame);
    _backBufferDirty = true;
}
#endif

//
// GPU
//

void FfmpegGPUVideoSwapChain::initialize(AVCodecContext* context) {
    
}

AVFrame* FfmpegGPUVideoSwapChain::getFrontBufferFrame() {
    return nullptr;
}

void FfmpegGPUVideoSwapChain::swap() {

}

service::recorder::image::Image FfmpegGPUVideoSwapChain::cpuCopyFrontBuffer() const {
    return service::recorder::image::Image{};
}

bool FfmpegGPUVideoSwapChain::hasValidFrontBuffer() const {
    return false;
}

bool FfmpegGPUVideoSwapChain::isSupported() {
    return false; // IsWindows8OrGreater();
}

size_t FfmpegGPUVideoSwapChain::frameWidth() const {
    return 0;
}

size_t FfmpegGPUVideoSwapChain::frameHeight() const {
    return 0;
}

void FfmpegGPUVideoSwapChain::receiveCpuFrame(const service::recorder::image::Image& frame) {

}

#ifdef _WIN32
void FfmpegGPUVideoSwapChain::receiveGpuFrame(ID3D11Texture2D* frame) {
    
}
#endif

}