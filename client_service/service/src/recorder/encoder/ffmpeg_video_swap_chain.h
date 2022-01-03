#pragma once

#include "recorder/image/image.h"
#include "recorder/image/d3d_image.h"
#include "renderer/d3d11_renderer.h"
#include "renderer/d3d11_video_processor.h"

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

    virtual size_t frameWidth() const = 0;
    virtual size_t frameHeight() const = 0;

    virtual AVFrame* receiveCpuFrame(const service::recorder::image::Image& frame) = 0;
#ifdef _WIN32
    virtual AVFrame* receiveGpuFrame(ID3D11Texture2D* frame) = 0;
#endif

protected:
    bool isGpuInit() const { return _shared; }
    service::renderer::D3d11SharedContext* _shared = nullptr;
};
using FfmpegVideoSwapChainPtr = std::unique_ptr<FfmpegVideoSwapChain>;

class FfmpegCPUVideoSwapChain: public FfmpegVideoSwapChain {
public:
    ~FfmpegCPUVideoSwapChain();

    void initialize(AVCodecContext* context, AVBufferRef* hwFrameCtx) override;

    size_t frameWidth() const override;
    size_t frameHeight() const override;

    AVFrame* receiveCpuFrame(const service::recorder::image::Image& frame) override;
#ifdef _WIN32
    AVFrame* receiveGpuFrame(ID3D11Texture2D* frame) override;
#endif
private:
    AVFrame* _frame = nullptr;
    AVFrame* process(const service::recorder::image::Image& input);

    service::recorder::image::ImagePtr _buffer;
    SwsContext* _sws = nullptr;
    void reinitBackBuffer(size_t width, size_t height);
};

class FfmpegGPUVideoSwapChain: public FfmpegVideoSwapChain {
public:
    static bool isSupported(service::renderer::D3d11SharedContext* shared, size_t width, size_t height);
    ~FfmpegGPUVideoSwapChain();
    
    void initialize(AVCodecContext* context, AVBufferRef* hwFrameCtx) override;

    size_t frameWidth() const override;
    size_t frameHeight() const override;

    AVFrame* receiveCpuFrame(const service::recorder::image::Image& frame) override;
#ifdef _WIN32
    AVFrame* receiveGpuFrame(ID3D11Texture2D* frame) override;
#endif

private:
    AVFrame* _frame = nullptr;
    AVFrame* process(ID3D11Texture2D* input);

    service::recorder::image::D3dImagePtr _buffer;
    void reinitBackBuffer(size_t width, size_t height);

    service::renderer::D3d11VideoProcessorPtr _processor;
};

}