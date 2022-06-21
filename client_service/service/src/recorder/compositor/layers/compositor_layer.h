#pragma once

#include <atomic>
#include <memory>

#include "renderer/d3d11_renderer.h"
#include "recorder/image/d3d_image.h"
#include "recorder/encoder/av_sync.h"

namespace service::recorder::compositor::layers {

class CompositorLayer {
public:
    virtual ~CompositorLayer() {}

    void enable(bool v) { _enabled = v; }
    virtual void updateAt(const service::recorder::encoder::AVSyncClock::time_point& tp, service::renderer::D3d11Renderer* renderer, ID3D11DeviceContext* context) = 0;
    virtual void finalizeAssetsForRenderer(service::renderer::D3d11Renderer* renderer) = 0;

protected:
    std::atomic<bool> _enabled = true;
};

using CompositorLayerPtr = std::shared_ptr<CompositorLayer>;

}