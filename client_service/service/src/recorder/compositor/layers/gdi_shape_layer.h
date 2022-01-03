#pragma once

#include "recorder/compositor/layers/compositor_layer.h"
#include "shared/color.h"
#include <variant>

namespace service::recorder::compositor::layers {

struct GdiBrushData {
    shared::color::ColorRgba fillColor;
};

struct GdiRectangleData {
    float left;
    float top;
    float right;
    float bottom;
};

using GdiShapeData = std::variant<GdiRectangleData>;

class GdiShapeLayer: public CompositorLayer {
public:
    GdiShapeLayer(const GdiShapeData& shape, const GdiBrushData& brush);
    ~GdiShapeLayer();

    void updateAt(const service::recorder::encoder::AVSyncClock::time_point& tp, service::renderer::D3d11Renderer* renderer) override {}

    void finalizeAssetsForRenderer(service::renderer::D3d11Renderer* renderer) override;
    void customRender(ID3D11Texture2D* output, IDXGISurface1* surface, HDC hdc) override;

private:
    GdiShapeData _shape;
    GdiBrushData _brush;
    HBRUSH _hbrush = 0;
};

}