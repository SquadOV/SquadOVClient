#pragma once

#include <stdint.h>
#include <filesystem>
#include <memory>

#ifdef _WIN32
#include <d3d11.h>
#endif

namespace service::recorder::image {

// General representation of an BGRA image that we'd want to use in our videos.
// We can just use BGRA images since that'll generalize to the vast majority of situations.
class Image {
public:
    Image() = default;
    Image(Image&& other);
    ~Image();

    uint8_t* buffer() { return _buffer.get(); }
    const uint8_t* buffer() const { return _buffer.get(); }
    
    size_t numBytes() const { return numBytesPerRow() * height(); }
    size_t numBytesPerRow() const { return width() * bytesPerPixel(); }
    size_t width() const { return _width; }
    size_t height() const { return _height; }
    size_t bytesPerPixel() const;

    void initializeImage(size_t width, size_t height, DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM);
    void copyFrom(const Image& img);
    void saveToFile(const std::filesystem::path& path) const;
    void loadFromFile(const std::filesystem::path& path);
    bool isInit() const { return _width > 0 && _height > 0; }

#ifdef _WIN32
    void loadFromD3d11Texture(ID3D11DeviceContext* context, ID3D11Texture2D* texture);
    void loadFromD3d11TextureWithStaging(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* texture);
#endif

    void fillAlpha(uint8_t v);
    DXGI_FORMAT format() const { return _format; }

private:
    size_t _width = 0;
    size_t _height = 0;
    std::unique_ptr<uint8_t[]> _buffer;

#ifdef _WIN32
    DXGI_FORMAT _format = DXGI_FORMAT_UNKNOWN;
    ID3D11Texture2D* _stagingTexture = nullptr;
#endif
};

using ImagePtr = std::unique_ptr<Image>;

}