#include "recorder/image/image.h"

#include <cstring>
#include <png.h>
#include "shared/errors/error.h"

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
        path.string().c_str(),
        0,
        _buffer.get(),
        0,
        NULL)) {
        THROW_ERROR("Failed to write PNG image to file.");
    }
}

}