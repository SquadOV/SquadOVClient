#pragma once

#include "recorder/compositor/layers/compositor_layer.h"
#include "renderer/d3d11_model.h"

namespace service::recorder::compositor::layers {

class MouseCursorLayer: public CompositorLayer {
public:
    explicit MouseCursorLayer(HWND parentWindow);

    void updateAt(const service::recorder::encoder::AVSyncClock::time_point& tp, service::renderer::D3d11Renderer* renderer) override {}
    void finalizeAssetsForRenderer(service::renderer::D3d11Renderer* renderer) override {}

    // The MouseCursorLayer will do its own rendering using GDI instead.
    void customRender(ID3D11Texture2D* output, IDXGISurface1* surface, HDC hdc) override;
private:
    HWND _parentWindow;
};

}