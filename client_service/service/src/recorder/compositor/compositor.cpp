#include "recorder/compositor/compositor.h"
#include "shared/log/log.h"

namespace service::recorder::compositor {

Compositor::Compositor(const service::renderer::D3d11Device device) {
    _device = device;
    _d3dContext = std::make_unique<service::renderer::D3d11SharedContext>(
        service::renderer::CONTEXT_FLAG_USE_D3D11_1,
        nullptr,
        device
    );
}

layers::ClockLayerPtr Compositor::createClockLayer() {
    if (_clockLayer) {
        LOG_WARNING("Only 1 clock layer can be created - ignoring" << std::endl);
        return nullptr;
    }

    auto layer = std::make_shared<layers::ClockLayer>();
    if (layer) {
        layer->setCallback(std::bind(&Compositor::tick, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }
    _clockLayer = layer;
    return layer;
}

void Compositor::tick(service::renderer::D3d11SharedContext* imageContext, ID3D11Texture2D* image, size_t numFrames) {
    if (!_clockLayer) {
        return;
    }
    assert(imageContext == context());

    // The output image needs to be the same resolution as the clock layer. Note
    // that the image we want to send will *always* come from a D3D11 texture (since we'll
    // use the WARP software rasterizer in the case of using the CPU).
    ID3D11Texture2D* texToSend = nullptr;

    // Note that every layer from the various layers must create a ID3D11Texture2D with the following properties:
    //  1) The texture must be shared (i.e. have the flag D3D11_RESOURCE_MISC_SHARED)
    //  2) The texture must be created on the same device/context as the compositor.
    if (_layers.empty()) {
        // Use the clock layer directly as the image to send to the encoder.
        texToSend = image;
    } else {
        // Use a renderer to combine the layers together with the clock layer.
    }

    // Finally send the image to the encoder. We could theoretically
    // hold the encoder mutex the whole time of the tick but I don't think
    // there's a need to optimize that. We will theoretically do extra work
    // if no encoder is set but that should be a rare state. Note that it's
    // the encoder's reponsibility for doing any color space transformations
    // and image resizing.
    if (texToSend) {
        std::lock_guard<std::mutex> guard(_encoderMutex);
        if (_activeEncoder) {
            _activeEncoder->addVideoFrame(texToSend, numFrames);
        }
    }
}

void Compositor::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder) {
    std::lock_guard<std::mutex> guard(_encoderMutex);
    _activeEncoder = encoder;
}

}