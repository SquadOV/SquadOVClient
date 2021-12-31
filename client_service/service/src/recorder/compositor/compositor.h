#pragma once

#include "recorder/compositor/layers/clock_layer.h"
#include "recorder/compositor/layers/compositor_layer.h"
#include "recorder/encoder/av_encoder.h"
#include "renderer/d3d11_context.h"

#include <memory>
#include <mutex>
#include <vector>

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
    explicit Compositor(const service::renderer::D3d11Device device);

    service::renderer::D3d11SharedContext* context() const { return _d3dContext.get(); }

    layers::ClockLayerPtr createClockLayer();

    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder);
    
private:
    layers::ClockLayerPtr _clockLayer;
    std::vector<layers::CompositorLayerPtr> _layers;

    service::renderer::D3d11Device _device;
    service::renderer::D3d11SharedContextPtr _d3dContext;

    service::recorder::encoder::AvEncoder* _activeEncoder = nullptr;
    std::mutex _encoderMutex;

    void tick(service::renderer::D3d11SharedContext* imageContext, ID3D11Texture2D* image, size_t numFrames);
};

using CompositorPtr = std::unique_ptr<Compositor>;

}