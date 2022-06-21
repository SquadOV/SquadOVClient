#include "recorder/compositor/layers/clock_layer.h"

namespace service::recorder::compositor::layers {

void ClockLayer::setSinkNode(const service::recorder::compositor::graph::SinkNodePtr& sink) {
    _sink = sink;
    if (_sink) {
        _sink->setCallback([this](service::renderer::D3d11SharedContext* imageContext, ID3D11Texture2D* image, size_t numFrames){
            if (_callback) {
                _callback(imageContext, image, numFrames);
            }
        });
    }
}

void ClockLayer::finalizeAssetsForRenderer(service::renderer::D3d11Renderer* renderer) {

}

void ClockLayer::updateAt(const service::recorder::encoder::AVSyncClock::time_point& tp, service::renderer::D3d11Renderer* renderer, ID3D11DeviceContext* context) {
}

}