#include "recorder/compositor/layers/overlay_layers.h"
#include "shared/log/log.h"
#include "recorder/compositor/layers/gdi_shape_layer.h"

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

            GdiBrushData brush;
            brush.fillColor = shared::color::ColorRgba::parseCssRgba(fill);

            const auto unnormalizedWidth = width * scaleX;
            const auto unnormalizedHeight = height * scaleY;

            GdiRectangleData rect;
            rect.left = (left - ((originX == "left") ? 0.f : unnormalizedWidth));
            rect.top = (top - ((originY == "top") ? 0.f : unnormalizedHeight));
            rect.right = rect.left + unnormalizedWidth;
            rect.bottom = rect.top + unnormalizedHeight;

            rect.left /= layer.width;
            rect.right /= layer.width;

            rect.top /= layer.height;
            rect.bottom /= layer.height;

            ret.push_back(std::make_shared<GdiShapeLayer>(rect, brush));
        } else {
            LOG_WARNING("Unsupported type for overlay: " << type << std::endl);
            continue;
        }

    }
    return ret;
}

}