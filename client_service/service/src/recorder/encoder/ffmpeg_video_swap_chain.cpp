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
    
    _frame->hw_frames_ctx = hwFrameCtx;
    _frame->format = context->pix_fmt;
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
    _frontBuffer = std::make_unique<service::recorder::image::D3dImage>(_shared);
    _backBuffer = std::make_unique<service::recorder::image::D3dImage>(_shared);
}

AVFrame* FfmpegGPUVideoSwapChain::getFrontBufferFrame() {
    std::lock_guard<std::mutex> guard(_frontMutex);

    if (_frontBuffer->isInit()) {
        // Copy from the front buffer to the frame's texture using D3D11's video processing capabilities
        // to ensure that we're able to convert to NV12 properly. Note that we have to obtain a shared
        // handle to the frame's texture since we let ffmpeg create its own D3d11DeviceContext to make sure
        // we aren't competing with ffmpeg for the context.
        auto* frameTexture = reinterpret_cast<ID3D11Texture2D*>(_frame->data[0]);

        IDXGIResource* frameResource = nullptr;
        HRESULT hr = frameTexture->QueryInterface(__uuidof(IDXGIResource), (void**)&frameResource);
        if (hr != S_OK) {
            THROW_ERROR("Failed to get frame DXGI resource.");
        }

        HANDLE frameHandle = nullptr;
        hr = frameResource->GetSharedHandle(&frameHandle);
        if (hr != S_OK) {
            frameResource->Release();
            THROW_ERROR("Failed to get frame shared handle.");
        }

        ID3D11Resource* sResource = nullptr;
        hr = _shared->device()->OpenSharedResource(frameHandle, __uuidof(ID3D11Resource), (void**)&sResource);
        if (hr != S_OK) {
            frameResource->Release();
            THROW_ERROR("Failed to open shared resource.");
        }

        ID3D11Texture2D* sTexture = nullptr;
        hr = sResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&sTexture);
        if (hr != S_OK) {
            frameResource->Release();
            sResource->Release();
            THROW_ERROR("Failed to get texture 2D from shared resource.");
        }

        _processor->process(_frontBuffer->rawTexture(), sTexture);
        
        frameResource->Release();
        sResource->Release();
        sTexture->Release();
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

bool FfmpegGPUVideoSwapChain::isSupported() {
    return IsWindows8OrGreater();
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
        _backBuffer->initializeImage(width, height);
    }
}

void FfmpegGPUVideoSwapChain::receiveCpuFrame(const service::recorder::image::Image& frame) {
    std::lock_guard<std::mutex> guard(_backMutex);
    reinitBackBuffer(frame.width(), frame.height());
    _backBuffer->copyFromCpu(frame);
    _backBufferDirty = true;
}

#ifdef _WIN32
void FfmpegGPUVideoSwapChain::receiveGpuFrame(ID3D11Texture2D* frame) {
    D3D11_TEXTURE2D_DESC desc;
    frame->GetDesc(&desc);

    std::lock_guard<std::mutex> guard(_backMutex);
    reinitBackBuffer(desc.Width, desc.Height);
    _backBuffer->copyFromGpu(frame);
    _backBufferDirty = true;
}
#endif

}