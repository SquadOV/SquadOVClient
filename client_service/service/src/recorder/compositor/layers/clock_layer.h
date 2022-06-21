#pragma once

#include "recorder/compositor/layers/compositor_layer.h"
#include "recorder/compositor/graph/sink_node.h"

#include <functional>

namespace service::recorder::compositor::layers {

using ClockLayerCallback = std::function<void(service::renderer::D3d11SharedContext*, ID3D11Texture2D*, size_t)>;

class ClockLayer: public CompositorLayer {
public:
    void setCallback(const ClockLayerCallback& cb) { _callback = cb; }
    void setSinkNode(const service::recorder::compositor::graph::SinkNodePtr& sink);

    void updateAt(const service::recorder::encoder::AVSyncClock::time_point& tp, service::renderer::D3d11Renderer* renderer, ID3D11DeviceContext* context) override;
    void finalizeAssetsForRenderer(service::renderer::D3d11Renderer* renderer) override;
    
private:
    ClockLayerCallback _callback;
    service::recorder::compositor::graph::SinkNodePtr _sink;
};

using ClockLayerPtr = std::shared_ptr<ClockLayer>;

}