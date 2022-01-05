#include "recorder/compositor/graph/sink_node.h"
#include <cassert>

namespace service::recorder::compositor::graph {

void SinkNode::receiveTexture(service::renderer::D3d11SharedContext* imageContext, ID3D11Texture2D* image, size_t numFrames, DXGI_MODE_ROTATION rotation) {
    assert(rotation == DXGI_MODE_ROTATION_IDENTITY);
    if (_callback) {
        _callback(imageContext, image, numFrames);
    }
}

}