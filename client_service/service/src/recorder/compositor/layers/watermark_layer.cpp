#include "recorder/compositor/layers/watermark_layer.h"
#include "shared/filesystem/common_paths.h"
#include "shared/log/log.h"
#include "shared/http/http_client.h"

#include <boost/algorithm/string.hpp>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/files.h>
#include <cryptopp/md5.h>
#include <cryptopp/base64.h>

namespace fs = std::filesystem;
namespace service::recorder::compositor::layers {

std::optional<std::string> WatermarkLayer::_cachedWatermarkHash;

WatermarkLayer::WatermarkLayer(const service::system::WatermarkSettings& watermark):
    _watermark(watermark)
{
}

WatermarkLayer::~WatermarkLayer() {
}

void WatermarkLayer::initializeWatermark(service::renderer::D3d11Renderer* renderer) {
    // We should cache the watermark though (which does introduce some mechanisms of bypass). The only possible thing
    // we can do here is to cache it to a file and store an MD5 in memory and when we go back to load it up double check the hash.
    // Granted, a user could find and modify the MD5 hash in memory using a memory editor or something...but good for them.
    const auto cachedImagePath = shared::filesystem::getSquadOvTempFolder() / fs::path("watermark.png");
    
    size_t maxTryCounter = 0;
    while(!fs::exists(cachedImagePath) || !_cachedWatermarkHash || computeHash(cachedImagePath) != _cachedWatermarkHash.value()) {
        if (maxTryCounter >= 5) {
            // Yeah, if users somehow bypass this shit, just let it keep functioning rather than erroring out here.
            // I'd say it's not the end of the world.
            LOG_ERROR("Failed to initialize watermark.");
            return;
        }

        // Download into cache and store the Md5.
        _cachedWatermarkHash = downloadWatermarkTo(cachedImagePath);
        ++maxTryCounter;
    }

    // Now we need to create the standard GdiImageLayer.
    GdiImageData data;
    data.image = std::shared_ptr<Gdiplus::Image>(Gdiplus::Image::FromFile(cachedImagePath.c_str(), false));
    data.src = Gdiplus::RectF(0, 0, data.image->GetWidth(), data.image->GetHeight());
    data.dst = Gdiplus::RectF(
        (_watermark.xPos  == service::system::SquadOvPositionX::Left) ? 0.0 :
            (_watermark.xPos  == service::system::SquadOvPositionX::Right) ? 1.0 :
            0.5,
        (_watermark.yPos  == service::system::SquadOvPositionY::Top) ? 0.0 :
            (_watermark.yPos  == service::system::SquadOvPositionY::Bottom) ? 1.0 :
            0.5,
        _watermark.size * data.image->GetWidth() / data.image->GetHeight(),
        _watermark.size
    );
    data.dstOffsetX = (_watermark.xPos  == service::system::SquadOvPositionX::Left) ? 0.0 :
        (_watermark.xPos  == service::system::SquadOvPositionX::Right) ? -1.0 :
        -0.5;
    data.dstOffsetY = (_watermark.yPos  == service::system::SquadOvPositionY::Top) ? 0.0 :
        (_watermark.yPos  == service::system::SquadOvPositionY::Bottom) ? -1.0 :
        -0.5;
    data.alpha = 0.3;
    _imageLayer = std::make_unique<GdiImageLayer>(data);
}

std::string WatermarkLayer::downloadWatermarkTo(const std::filesystem::path& fname) const {
    // Load the watermark from a URL. This way we generally don't have to worry about the user tampering with
    // a file on their local machine and by-passing the watermark. The file should be small enough that I don't need
    // to worry that the user's internet will be severely impacted by retrieving this file.
    const auto watermarkUrl = "https://us-central1.content.squadov.gg/SquadOV_Gradient_White.png";
    if (!shared::http::downloadFileToLocation(watermarkUrl, fname, [](size_t, size_t, size_t, size_t){})) {
        return "";
    }
    return computeHash(fname);
}

std::string WatermarkLayer::computeHash(const std::filesystem::path& fname) const {
    if (!fs::exists(fname)) {
        return "";
    }
    CryptoPP::Weak::MD5 hash;
    std::string digest;
    
    CryptoPP::FileSource file(
        fname.c_str(),
        true,
        new CryptoPP::HashFilter(hash, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(digest)))
    );

    boost::algorithm::trim(digest);
    return digest;
}

void WatermarkLayer::updateAt(const service::recorder::encoder::AVSyncClock::time_point& tp, service::renderer::D3d11Renderer* renderer) {
    if (!_imageLayer) {
        return;
    }
    _imageLayer->updateAt(tp, renderer);
}

void WatermarkLayer::finalizeAssetsForRenderer(service::renderer::D3d11Renderer* renderer) {
    initializeWatermark(renderer);

    if (!_imageLayer) {
        return;
    }
    _imageLayer->finalizeAssetsForRenderer(renderer);
}

void WatermarkLayer::customRender(ID3D11Texture2D* output, IDXGISurface1* surface, HDC hdc) {
    if (!_imageLayer) {
        return;
    }
    _imageLayer->customRender(output, surface, hdc);
}

}