#pragma once

#include "recorder/compositor/layers/image_layer.h"
#include "recorder/compositor/graph/sink_node.h"

#include <functional>

namespace service::recorder::compositor::layers {

using ClockLayerCallback = std::function<void(service::renderer::D3d11SharedContext*, ID3D11Texture2D*, size_t)>;

class ClockLayer: public ImageLayer {
public:
    ID3D11Texture2D* get() const override;

    void setCallback(const ClockLayerCallback& cb) { _callback = cb; }
    void setSinkNode(const service::recorder::compositor::graph::SinkNodePtr& sink);

private:
    ClockLayerCallback _callback;
    service::recorder::compositor::graph::SinkNodePtr _sink;
};

using ClockLayerPtr = std::shared_ptr<ClockLayer>;

}