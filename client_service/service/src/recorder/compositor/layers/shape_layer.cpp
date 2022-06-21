#include "recorder/compositor/layers/shape_layer.h"
#include "shared/log/log.h"

#include <WICTextureLoader.h>

namespace service::recorder::compositor::layers {

ShapeLayer::ShapeLayer(const ShapeData& shape):
    _shape(shape)
{

}

void ShapeLayer::initializeImage(service::renderer::D3d11Renderer* renderer, const std::filesystem::path& fname) {
    HRESULT hr = DirectX::CreateWICTextureFromFileEx(
        renderer->device(),                 // d3dDevice
        fname.c_str(),                      // szFileName
        0,                                  // maxsize
        D3D11_USAGE_DEFAULT,                // usage
        D3D11_BIND_SHADER_RESOURCE,         // bindflags
        0,                                  // cpuAccessFlags
        0,                                  // miscFlags
        DirectX::WIC_LOADER_DEFAULT,        // loadFlags
        (ID3D11Resource**)_texture.put(),   // texture
        nullptr                             // textureView
    );

    if (hr != S_OK) {
        LOG_ERROR("Failed to initialize image layer from file: " << fname << std::endl);
        return;
    }

    _texture->GetDesc(&_desc);
}

void ShapeLayer::updateAt(const service::recorder::encoder::AVSyncClock::time_point& tp, service::renderer::D3d11Renderer* renderer, ID3D11DeviceContext* context) {
    _model->clearXform();
    _model->setTranslation(_translation);
    _model->setScale(_scale);
    _model->setOpacity(_opacity);

    if (_sentTexture) {
        return;
    }

    _model->setTexture(renderer->device(), context, _texture.get(), true);
    _sentTexture = true;
}

void ShapeLayer::finalizeAssetsForRenderer(service::renderer::D3d11Renderer* renderer) {
    switch (_shape.shape) {
        case Shape::Rectangle:
            if (_texture) {
                _model = service::renderer::createFullScreenQuad(renderer->device());
            } else {
                _model = service::renderer::createSolidColorQuad(
                    renderer->device(),
                    DirectX::XMFLOAT2(0.f, 0.f),
                    DirectX::XMFLOAT2(1.f, 1.f),
                    DirectX::XMFLOAT4(_shape.fillColor.r, _shape.fillColor.g, _shape.fillColor.b, _shape.fillColor.a)
                );
            }
            break;
    }
    renderer->addModelToScene(_model);
}

void ShapeLayer::setOpacity(float val) {
    _opacity = val;
}

void ShapeLayer::setScale(DirectX::XMFLOAT3 scale) {
    _scale = scale;
}

void ShapeLayer::setTranslation(DirectX::XMFLOAT3 delta) {
    _translation = delta;
}

size_t ShapeLayer::width() const {
    return _desc.Width;
}

size_t ShapeLayer::height() const {
    return _desc.Height;
}

}