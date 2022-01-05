#pragma once

#include "recorder/compositor/graph/compositor_node.h"
#include "recorder/image/d3d_image.h"
#include "recorder/image/image.h"
#include "renderer/d3d11_context.h"

namespace service::recorder::compositor::graph {

class TextureContextNormalizerNode: public CompositorNode {
public:
    explicit TextureContextNormalizerNode(service::renderer::D3d11SharedContext* context);

protected:
    void receiveTexture(service::renderer::D3d11SharedContext* imageContext, ID3D11Texture2D* image, size_t numFrames, DXGI_MODE_ROTATION rotation) override;
    
private:
    service::renderer::D3d11SharedContext* _context;
    service::recorder::image::D3dImagePtr _newImage;
    service::recorder::image::ImagePtr _tmpImage;
};

using TextureContextNormalizerNodePtr = std::shared_ptr<TextureContextNormalizerNode>;

}