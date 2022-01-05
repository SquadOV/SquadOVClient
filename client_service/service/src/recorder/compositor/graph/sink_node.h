#pragma once

#include "recorder/compositor/graph/compositor_node.h"

#include <functional>

namespace service::recorder::compositor::graph {

using SinkNodeCallback = std::function<void(service::renderer::D3d11SharedContext*, ID3D11Texture2D*, size_t)>;

class SinkNode: public CompositorNode {
public:
    void setCallback(const SinkNodeCallback& cb) { _callback = cb; }

protected:
    void receiveTexture(service::renderer::D3d11SharedContext* imageContext, ID3D11Texture2D* image, size_t numFrames, DXGI_MODE_ROTATION rotation) override;

private:
    SinkNodeCallback _callback;
};

using SinkNodePtr = std::shared_ptr<SinkNode>;

}