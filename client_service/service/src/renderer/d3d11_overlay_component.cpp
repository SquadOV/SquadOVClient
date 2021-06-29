#include "renderer/d3d11_overlay_component.h"
#include "shared/log/log.h"

namespace service::renderer {
    
D3d11OverlayComponentPtr D3d11OverlayComponent::loadFromJson(const nlohmann::json& data, size_t width, size_t height) {
    auto component = std::make_shared<D3d11OverlayComponent>(width, height);

    const auto type = data["type"].get<std::string>();
    if (type == "rect") {
        const auto left = data.value("left", 0.0f);
        const auto top = data.value("top", 0.0f);
        const auto width = data.value("width", 0.0f);
        const auto height = data.value("height", 0.0f);
        const auto scaleX = data.value("scaleX", 0.0f);
        const auto scaleY = data.value("scaleY", 0.0f);
        const auto fill = data.value("fill", "rgba(0, 0, 0, 0)");

        RectangleData rectangle;
        rectangle.left = left;
        rectangle.top = top;
        rectangle.width = width;
        rectangle.height = height;
        rectangle.scaleX = scaleX;
        rectangle.scaleY = scaleY;
        rectangle.color = shared::color::ColorRgba::parseCssRgba(fill);

        component->_rectangle = rectangle;
    } else {
        LOG_WARNING("Unsupported type for overlay: " << type << std::endl);
        return nullptr;
    }

    return component;
}

D3d11OverlayComponent::D3d11OverlayComponent(size_t width, size_t height):
    _canvasWidth(width),
    _canvasHeight(height)
{

}

void D3d11OverlayComponent::addToRenderer(D3d11Renderer& renderer, D3d11SharedContext& context) {
    if (!_cachedModel) {
        if (_rectangle) {
            const auto rectangle = _rectangle.value();
            _cachedModel = createSolidColorQuad(
                context.device(),
                DirectX::XMFLOAT2(
                    rectangle.left / _canvasWidth,
                    rectangle.top / _canvasHeight
                ),
                // TODO: Opt to do actual transformations instead...
                DirectX::XMFLOAT2(
                    rectangle.width * rectangle.scaleX / _canvasWidth,
                    rectangle.height * rectangle.scaleY / _canvasHeight
                ),
                DirectX::XMFLOAT4(rectangle.color.r, rectangle.color.b, rectangle.color.g, rectangle.color.a)
            );
        }
    }

    if (_cachedModel) {
        renderer.addModelToScene(_cachedModel);
    }
}

void D3d11OverlayComponent::clearCache() {
    _cachedModel.reset();
}

}