#include "recorder/compositor/layers/overlay_layers.h"

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
        layer.width = obj.value("width", 0);
    }

    {
        const auto it = obj.find("height");
        if (it != obj.end() && !it->is_null()) {
            layer.height = it->get<size_t>();
        } else {
            layer.height = 1080;
        }
        layer.height = obj.value("height", 0);
    }

    return layer;
}

std::vector<CompositorLayerPtr> createCompositorLayersForOverlayLayer(const OverlayLayer& layer) {
    return {};
}

}