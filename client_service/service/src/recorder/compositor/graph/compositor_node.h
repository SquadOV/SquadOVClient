#pragma once

#include "renderer/d3d11_context.h"
#include <d3d11.h>
#include <memory>

namespace service::recorder::compositor::graph {

class CompositorNode;
using CompositorNodePtr = std::shared_ptr<CompositorNode>;

class CompositorNode {
public:
    virtual ~CompositorNode() {}

    void setNext(const CompositorNodePtr& next);

protected:
    virtual void receiveTexture(service::renderer::D3d11SharedContext* imageContext, ID3D11Texture2D* image, size_t numFrames, DXGI_MODE_ROTATION rotation) {}
    void flowToNext(service::renderer::D3d11SharedContext* imageContext, ID3D11Texture2D* image, size_t numFrames = 1, DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_IDENTITY);

private:
    CompositorNodePtr _next;
};

}