#include "recorder/compositor/layers/gdi_shape_layer.h"

namespace service::recorder::compositor::layers {

GdiShapeLayer::GdiShapeLayer(const GdiShapeData& shape, const GdiBrushData& brush):
    _shape(shape),
    _brush(brush)
{
}

GdiShapeLayer::~GdiShapeLayer() {
    if (_hbrush) {
        DeleteObject(_hbrush);
    }
}

void GdiShapeLayer::finalizeAssetsForRenderer(service::renderer::D3d11Renderer* renderer) {
    _hbrush = CreateSolidBrush(_brush.fillColor.toGdiColor());
}

void GdiShapeLayer::customRender(ID3D11Texture2D* output, IDXGISurface1* surface, HDC hdc) {
    SelectObject(hdc, _hbrush);

    // Note that the shapes will be given in normalized coordinates. Need to get
    // the output texture's resolution to unnormalize as per GDI's requirements.
    D3D11_TEXTURE2D_DESC desc;
    output->GetDesc(&desc);

    if (std::holds_alternative<GdiRectangleData>(_shape)) {
        const auto& rect = std::get<GdiRectangleData>(_shape);
        Rectangle(hdc, rect.left * desc.Width, rect.top * desc.Height, rect.right * desc.Width, rect.bottom * desc.Height);
    }

    SelectObject(hdc, nullptr);
}

}