#pragma once

#include "recorder/compositor/layers/clock_layer.h"
#include "recorder/compositor/layers/compositor_layer.h"
#include "recorder/encoder/av_encoder.h"
#include "renderer/d3d11_context.h"
#include "renderer/d3d11_renderer.h"
#include "renderer/d3d11_model.h"

#include <memory>
#include <mutex>
#include <vector>
#include <wil/com.h>
#include <DirectXTK/PostProcess.h>

namespace service::recorder::compositor {

// The compositor is responsible for combining multiple layers
// together into a single image and shipping off the final image to the
// encoder for the final video.
//
// The question then, is WHEN does the image get sent to the encoder?
// In this case, we opt for a design where the image is sent whenever a new
// image comes in via the "clock layer" (a push design rather than a pull design).
// Encoding is a potentially expensive process so we do not want to encode a
// new frame when no new frame is actually ready (note that our primary use case is
// in the context of screen recording). Thus, it makes more sense to send a new
// frame to the encoder only when we receive a new frame from whatever API we're using
// to record the screen. Therefore, whenever the compositor receives a new frame
// from the "clock layer", it'll use that layer as the base and composite the other
// layers on top. Therefore, only 1 layer will be "pushing" images; all the other
// layers will have images "pulled" from them. Note that if there's only 1 layer
// then we can do an optimization and not do a render call.
class Compositor {
public:
    explicit Compositor(const service::renderer::D3d11Device device, size_t width, size_t height);

    const service::renderer::D3d11SharedContextPtr& context() const { return _d3dContext; }

    layers::ClockLayerPtr createClockLayer();
    void addLayer(const layers::CompositorLayerPtr& layer);
    void finalizeLayers();

    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder);
    
private:
    layers::ClockLayerPtr _clockLayer;
    std::vector<layers::CompositorLayerPtr> _layers;

    service::renderer::D3d11Device _device;
    service::renderer::D3d11SharedContextPtr _d3dContext;

    service::recorder::encoder::AvEncoder* _activeEncoder = nullptr;
    std::mutex _encoderMutex;

    size_t _width = 0;
    size_t _height = 0;

    // DirectX rendering state
    service::renderer::D3d11RendererPtr _renderer;
    service::renderer::D3d11ModelPtr _fsQuad;

    // The output texture that will be rendered to by the renderer (this is the resize + layers step).
    wil::com_ptr<ID3D11Texture2D> _outputTexture;
    wil::com_ptr<ID3D11RenderTargetView> _outputRenderTarget;

    // The output texture that will be rendered to by the tonemapper.
    std::unique_ptr<DirectX::ToneMapPostProcess> _tonemapper;
    wil::com_ptr<ID3D11Texture2D> _tmTexture;
    wil::com_ptr<ID3D11RenderTargetView> _tmRenderTarget;

    void tick(service::renderer::D3d11SharedContext* imageContext, ID3D11Texture2D* image, size_t numFrames);
    void reinitOutputTexture(ID3D11Texture2D* image, const D3D11_TEXTURE2D_DESC& inputDesc);
    void reinitTonemapper(ID3D11Texture2D* image, const D3D11_TEXTURE2D_DESC& inputDesc);
};

using CompositorPtr = std::unique_ptr<Compositor>;

}