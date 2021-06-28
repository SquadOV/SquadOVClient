#pragma once

#include "recorder/image/image.h"
#include "recorder/image/d3d_image.h"
#include "renderer/d3d11_renderer.h"
#include "renderer/d3d11_video_processor.h"
#include "renderer/d3d11_overlay_renderer.h"

#include <memory>
#include <mutex>

#ifdef _WIN32
#include <Windows.h>
#include <d3d11.h>
#endif

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

namespace service::recorder::encoder {

class FfmpegVideoSwapChain {
public:
    virtual ~FfmpegVideoSwapChain() {}

    virtual void initialize(AVCodecContext* context, AVBufferRef* hwFrameCtx) = 0;
    void initializeGpuSupport(service::renderer::D3d11SharedContext* shared) { _shared = shared; }
    void initializeOverlay(const service::renderer::D3d11OverlayRendererPtr& overlay) { _overlay = overlay; }

    virtual AVFrame* getFrontBufferFrame() = 0;
    virtual service::recorder::image::Image cpuCopyFrontBuffer() const = 0;
    virtual bool hasValidFrontBuffer() const = 0;
    virtual void swap() = 0;

    virtual size_t frameWidth() const = 0;
    virtual size_t frameHeight() const = 0;

    virtual void receiveCpuFrame(const service::recorder::image::Image& frame) = 0;
#ifdef _WIN32
    virtual void receiveGpuFrame(ID3D11Texture2D* frame) = 0;
#endif

protected:
    bool isGpuInit() const { return _shared; }
    service::renderer::D3d11SharedContext* _shared = nullptr;
    service::renderer::D3d11OverlayRendererPtr _overlay;
};
using FfmpegVideoSwapChainPtr = std::unique_ptr<FfmpegVideoSwapChain>;

class FfmpegCPUVideoSwapChain: public FfmpegVideoSwapChain {
public:
    ~FfmpegCPUVideoSwapChain();

    void initialize(AVCodecContext* context, AVBufferRef* hwFrameCtx) override;

    AVFrame* getFrontBufferFrame() override;
    service::recorder::image::Image cpuCopyFrontBuffer() const override;
    bool hasValidFrontBuffer() const override;
    void swap() override;

    size_t frameWidth() const override;
    size_t frameHeight() const override;

    void receiveCpuFrame(const service::recorder::image::Image& frame) override;
#ifdef _WIN32
    void receiveGpuFrame(ID3D11Texture2D* frame) override;
#endif
private:
    AVFrame* _frame = nullptr;
    mutable std::mutex _frontMutex;
    service::recorder::image::ImagePtr _frontBuffer;
    SwsContext* _frontSws = nullptr;

    void reinitBackBuffer(size_t width, size_t height);
    mutable std::mutex _backMutex;
    service::recorder::image::ImagePtr _backBuffer;
    SwsContext* _backSws = nullptr;
    bool _backBufferDirty = false;
};

class FfmpegGPUVideoSwapChain: public FfmpegVideoSwapChain {
public:
    static bool isSupported(service::renderer::D3d11SharedContext* shared, size_t width, size_t height);
    ~FfmpegGPUVideoSwapChain();
    
    void initialize(AVCodecContext* context, AVBufferRef* hwFrameCtx) override;
    AVFrame* getFrontBufferFrame() override;
    service::recorder::image::Image cpuCopyFrontBuffer() const override;
    bool hasValidFrontBuffer() const override;
    void swap() override;

    size_t frameWidth() const override;
    size_t frameHeight() const override;

    void receiveCpuFrame(const service::recorder::image::Image& frame) override;
#ifdef _WIN32
    void receiveGpuFrame(ID3D11Texture2D* frame) override;
#endif

private:
    AVFrame* _frame = nullptr;
    mutable std::mutex _frontMutex;
    service::recorder::image::D3dImagePtr _frontBuffer;

    mutable std::mutex _backMutex;
    service::recorder::image::D3dImagePtr _backBuffer;
    bool _backBufferDirty = false;

    void reinitBackBuffer(size_t width, size_t height);
    service::renderer::D3d11VideoProcessorPtr _processor;
};

}