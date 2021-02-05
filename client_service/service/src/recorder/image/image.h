#pragma once

#include <stdint.h>
#include <filesystem>
#include <memory>

namespace service::recorder::image {

// General representation of an BGRA image that we'd want to use in our videos.
// We can just use BGRA images since that'll generalize to the vast majority of situations.
class Image {
public:
    uint8_t* buffer() { return _buffer.get(); }
    const uint8_t* buffer() const { return _buffer.get(); }
    
    size_t numBytes() const { return numBytesPerRow() * height(); }
    size_t numBytesPerRow() const { return width() * bytesPerPixel(); }
    size_t width() const { return _width; }
    size_t height() const { return _height; }
    size_t bytesPerPixel() const { return 4; }

    void initializeImage(size_t width, size_t height);
    void copyFrom(const Image& img);
    void saveToFile(const std::filesystem::path& path) const;
    bool isInit() const { return _width > 0 && _height > 0; }

    void fillAlpha(uint8_t v);

private:
    size_t _width = 0;
    size_t _height = 0;
    std::unique_ptr<uint8_t[]> _buffer;
};

using ImagePtr = std::unique_ptr<Image>;

}