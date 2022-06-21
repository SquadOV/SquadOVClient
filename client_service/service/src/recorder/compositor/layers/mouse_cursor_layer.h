#pragma once

#include "recorder/compositor/layers/compositor_layer.h"
#include "renderer/d3d11_model.h"

#include <atomic>
#include <mutex>
#include <thread>

namespace service::recorder::compositor::layers {

class MouseCursorLayer: public CompositorLayer {
public:
    explicit MouseCursorLayer(HWND parentWindow);

    void updateAt(const service::recorder::encoder::AVSyncClock::time_point& tp, service::renderer::D3d11Renderer* renderer, ID3D11DeviceContext* context) override;
    void finalizeAssetsForRenderer(service::renderer::D3d11Renderer* renderer) override;
private:
    HWND _parentWindow;

    wil::com_ptr<ID3D11Texture2D> _texture;
    wil::com_ptr<IDXGISurface1> _surface;
    D3D11_TEXTURE2D_DESC _desc = { 0 };
    service::renderer::D3d11ModelPtr _model;
};

}