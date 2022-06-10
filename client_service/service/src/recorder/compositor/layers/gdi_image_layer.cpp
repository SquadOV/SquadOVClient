#include "recorder/compositor/layers/gdi_image_layer.h"
#include "shared/log/log.h"

namespace service::recorder::compositor::layers {

GdiImageLayer::GdiImageLayer(const GdiImageData& image):
    _image(image)
{
}

GdiImageLayer::~GdiImageLayer() {
}

void GdiImageLayer::finalizeAssetsForRenderer(service::renderer::D3d11Renderer* renderer) {
}

void GdiImageLayer::customRender(ID3D11Texture2D* output, IDXGISurface1* surface, HDC hdc) {
    if (!_image.image) {
        return;
    }

    D3D11_TEXTURE2D_DESC desc;
    output->GetDesc(&desc);

    const auto newHeight = std::min(static_cast<UINT>(_image.dst.Height * desc.Height), _image.image->GetHeight());
    const auto newWidth = static_cast<UINT>(newHeight * static_cast<double>(_image.image->GetWidth()) / _image.image->GetHeight());

    Gdiplus::ImageAttributes attributes;
    Gdiplus::ColorMatrix colorMatrix = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 0.0f, static_cast<Gdiplus::REAL>(_image.alpha), 0.0f,
                                        0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    attributes.SetColorMatrix(
        &colorMatrix,
        Gdiplus::ColorMatrixFlagsDefault,
        Gdiplus::ColorAdjustTypeBitmap
    );

    std::unique_ptr<Gdiplus::Graphics> graphics(Gdiplus::Graphics::FromHDC(hdc));
    if (Gdiplus::Status::Ok != graphics->DrawImage(
        _image.image.get(),
        Gdiplus::RectF(
            _image.dst.X * desc.Width + _image.dstOffsetX * newWidth,
            _image.dst.Y * desc.Height + _image.dstOffsetY * newHeight,
            newWidth,
            newHeight
        ),
        _image.src,
        Gdiplus::Unit::UnitPixel,
        &attributes
    )) {
        LOG_WARNING("Failed to draw image." << std::endl);
    }
}

}