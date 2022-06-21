#pragma once

#include "recorder/compositor/layers/compositor_layer.h"
#include "renderer/d3d11_renderer.h"
#include "shared/color.h"

#include <filesystem>
#include <memory>
#include <wil/com.h>

namespace service::recorder::compositor::layers {

enum class Shape {
    Rectangle
};

struct ShapeData {
    Shape shape;
    shared::color::ColorRgba fillColor;
};

class ShapeLayer: public CompositorLayer {
public:
    explicit ShapeLayer(const ShapeData& shape);
    void initializeImage(service::renderer::D3d11Renderer* renderer, const std::filesystem::path& fname);

    void updateAt(const service::recorder::encoder::AVSyncClock::time_point& tp, service::renderer::D3d11Renderer* renderer, ID3D11DeviceContext* context) override;
    void finalizeAssetsForRenderer(service::renderer::D3d11Renderer* renderer) override;

    void setOpacity(float val);
    void setScale(DirectX::XMFLOAT3 scale);
    void setTranslation(DirectX::XMFLOAT3 delta);

    size_t width() const;
    size_t height() const;

private:
    ShapeData _shape;
    bool _sentTexture = false;
    wil::com_ptr<ID3D11Texture2D> _texture;
    wil::com_ptr<IDXGISurface1> _surface;
    service::renderer::D3d11ModelPtr _model;
    D3D11_TEXTURE2D_DESC _desc;

    float _opacity = 1.f;
    DirectX::XMFLOAT3 _scale;
    DirectX::XMFLOAT3 _translation;
};
using ShapeLayerPtr = std::shared_ptr<ShapeLayer>;

}