#pragma once

#include <windows.h>
#define GDIPVER 0x0110
#include <gdiplus.h>
#include <memory>

#include "recorder/compositor/layers/compositor_layer.h"

namespace service::recorder::compositor::layers {

struct GdiImageData {
    std::shared_ptr<Gdiplus::Image> image;
    Gdiplus::RectF src;

    // dst is in percentages of the output image instead of in pixel units.
    Gdiplus::RectF dst;

    double dstOffsetX = 0.0;
    double dstOffsetY = 0.0;
    double alpha = 1.0;
};

class GdiImageLayer: public CompositorLayer {
public:
    explicit GdiImageLayer(const GdiImageData& image);
    ~GdiImageLayer();

    void updateAt(const service::recorder::encoder::AVSyncClock::time_point& tp, service::renderer::D3d11Renderer* renderer) override {}

    void finalizeAssetsForRenderer(service::renderer::D3d11Renderer* renderer) override;
    void customRender(ID3D11Texture2D* output, IDXGISurface1* surface, HDC hdc) override;

private:
    GdiImageData _image;
};

}