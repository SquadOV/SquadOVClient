#include "recorder/image/image.h"

#include <cstring>
#include <png.h>
#include "shared/errors/error.h"
#include "shared/filesystem/utility.h"

#include <iostream>

namespace service::recorder::image {

void Image::initializeImage(size_t width, size_t height) {
    _width = width;
    _height = height;
    
    _buffer.reset(new uint8_t[numBytes()]);
    memset(_buffer.get(), 0, numBytes());
}

void Image::copyFrom(const Image& img) {
    assert(width() == img.width());
    assert(height() == img.height());
    assert(bytesPerPixel() == img.bytesPerPixel());
    std::memcpy(_buffer.get(), img.buffer(), numBytes());
}

void Image::fillAlpha(uint8_t v) {
    for (auto h = 0; h < height(); ++h) {
        for (auto w = 0; w < width(); ++w) {
            const auto idx = h * numBytesPerRow() + w * bytesPerPixel() + 3;
            _buffer[idx] = v;
        }
    }
}

void Image::saveToFile(const std::filesystem::path& path) const {
    png_image image;
    memset(&image, 0, sizeof(png_image));
    
    image.version = PNG_IMAGE_VERSION;
    image.opaque = NULL;
    image.width = static_cast<int>(width());
    image.height = static_cast<int>(height());
    image.format = PNG_FORMAT_BGRA;

    if (!png_image_write_to_file(
        &image,
        shared::filesystem::pathUtf8(path).c_str(),
        0,
        _buffer.get(),
        0,
        NULL)) {
        throw std::runtime_error("Failed to write PNG image to file.");
    }
}

}