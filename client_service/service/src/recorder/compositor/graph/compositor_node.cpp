#include "recorder/compositor/graph/compositor_node.h"

namespace service::recorder::compositor::graph {

void CompositorNode::setNext(const CompositorNodePtr& next) {
    _next = next;
}

void CompositorNode::flowToNext(service::renderer::D3d11SharedContext* imageContext, ID3D11Texture2D* image, size_t numFrames, DXGI_MODE_ROTATION rotation) {
    if (_next) {
        _next->receiveTexture(imageContext, image, numFrames, rotation);
    }
}

}