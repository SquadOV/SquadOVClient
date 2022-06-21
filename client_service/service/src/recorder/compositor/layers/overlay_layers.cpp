#include "recorder/compositor/layers/overlay_layers.h"
#include "recorder/compositor/layers/shape_layer.h"
#include "shared/log/log.h"

namespace service::recorder::compositor::layers {

OverlaySettings OverlaySettings::fromJson(const nlohmann::json& obj) {
    OverlaySettings settings;
    settings.enabled = obj.value("enabled", false);

    if (obj.count("layers") > 0) {
        for (const auto& val : obj["layers"]) {
            settings.layers.push_back(OverlayLayer::fromJson(val));
        }
    }
    return settings;
}

OverlayLayer OverlayLayer::fromJson(const nlohmann::json& obj) {
    OverlayLayer layer;
    layer.enabled = obj.value("enabled", false);
    layer.name = obj.value("name", "");
    if (obj.count("games") > 0) {
        for (const auto& val : obj["games"]) {
            layer.games.insert(static_cast<shared::EGame>(val.get<int32_t>()));
        }
    }
    layer.fabric = obj["fabric"];

    {
        const auto it = obj.find("width");
        if (it != obj.end() && !it->is_null()) {
            layer.width = it->get<size_t>();
        } else {
            layer.width = 1920;
        }
    }

    {
        const auto it = obj.find("height");
        if (it != obj.end() && !it->is_null()) {
            layer.height = it->get<size_t>();
        } else {
            layer.height = 1080;
        }
    }

    return layer;
}

std::vector<CompositorLayerPtr> createCompositorLayersForOverlayLayer(const OverlayLayer& layer) {
    if (!layer.fabric.contains("objects") || layer.fabric.count("objects") == 0) {
        return {};
    }

    std::vector<CompositorLayerPtr> ret;
    const auto& objects = layer.fabric["objects"];
    for (const auto& data : objects) {
        const auto type = data["type"].get<std::string>();

        if (type == "rect") {
            const auto left = data.value("left", 0.0f);
            const auto top = data.value("top", 0.0f);
            const auto width = data.value("width", 0.0f);
            const auto height = data.value("height", 0.0f);
            const auto scaleX = data.value("scaleX", 0.0f);
            const auto scaleY = data.value("scaleY", 0.0f);
            const auto fill = data.value("fill", "rgba(0, 0, 0, 0)");
            const auto originX = data.value("originX", "left");
            const auto originY = data.value("originY", "top");

            // This is all computations done in the "layer" space.
            // Regardless of origin, we want to change the rectangle into having an origin of "center" + "center"
            // since that'll correspond to how we render things.
            const auto layerWidth = width * scaleX;
            const auto layerHeight = height * scaleY;
            const auto layerX = left - ((originX == "left") ? 0.f : layerWidth) + layerWidth / 2.f;
            const auto layerY = top - ((originY == "top") ? 0.f : layerHeight) + layerHeight / 2.f;

            ShapeData shape;
            shape.shape = Shape::Rectangle;
            shape.fillColor = shared::color::ColorRgba::parseCssRgba(fill);

            auto newLayer = std::make_shared<ShapeLayer>(shape);

            // Change the previous "layer" space coordinates and normalize them into NDC.
            const auto ndcWidthScale = layerWidth / layer.width;
            const auto ndcHeightScale = layerHeight / layer.height;
            newLayer->setScale(DirectX::XMFLOAT3(ndcWidthScale, ndcHeightScale, 1.f));

            const auto ndcX = layerX / layer.width * 2.f - 1.f;;
            const auto ndcY = layerY / layer.height * -2.f + 1.f;
            newLayer->setTranslation(DirectX::XMFLOAT3(ndcX, ndcY, 0.f));
            
            ret.push_back(newLayer);
        } else {
            LOG_WARNING("Unsupported type for overlay: " << type << std::endl);
            continue;
        }

    }
    return ret;
}

}